#include "Feature.h"

#include "Damage.h"
#include "Debug.h"
#include "Draw.h"
#include "Item.h"
#include "Pathfind.h"
#include "Player.h"
#include "Random.h"
#include "Serialize.h"
#include "Terrain.h"
#include "VectorUtil.h"
#include "World.h"

namespace Feature
{

//-------------------------------------------------------------------------------------------------
// Data

// Feature Types:
//  - Chest - payload is Item::Handle
//  - Desk - no payload
//  - TorchUnlit - payload is which trigger id it activates
// 	             - trigger only activates when the last torch with that trigger is lit
//  - TorchLit - no payload
//  - Portrait - no payload
//  - FlipendoButton - payload is which trigger id it activates
//                   - it also turns into a wall on that trigger
//  - SlidingWall - payload is which trigger id it responds to
//  - Portcullis - payload is which trigger id it responds to

struct Instance
{
	Vec3 pos;
	int hp;

	// Parameters to be interpreted based on type of feature.
	int payload;
};
std::vector<Feature::Instance> s_features;

int s_next_trigger_id = 0;
std::vector<Vec3> s_features_to_update;  // index to s_features

enum class Material
{
	Stone,
	Metal,
	Wood,
	Count,
};

float constexpr c_Resistances[(int)(Material::Count)][Damage::Type::Count] =
{
	//	Basic	ToLife	Fire	Acid
	{	0.0f,	0.0f,	0.0f,	0.0f,	},	// Stone
	{	0.0f,	0.0f,	0.0f,	1.0f,	},	// Metal
	{	1.0f,	0.0f,	2.0f,	0.0f,	},	// Wood
};

//-------------------------------------------------------------------------------------------------
// Helper declarations

int find_feature(Vec3 pos);
void add_feature_internal(Vec3 pos, Terrain::Type terrain, int hp, int payload);

void register_for_updates(Feature::Instance& feature);
void init_chest(Feature::Instance& feature);
void init_desk(Feature::Instance& feature);

bool update_at(Vec3 pos);  // returns if there was a suitable feature to update
void update_scanner(Feature::Instance& feature);
void update_shop_seed(Feature::Instance& feature);

void damage_basic(Vec3 pos, Damage::Packet const& damage_packet,
                  Material const material, std::string const name);

void light_torch(Vec3 pos);
bool is_any_unlit_torch_with_trigger(int trigger);

void trigger_all(int trigger);  // invalidates all feature references including indexes
void trigger_remove_yourself(Feature::Instance & feature);
void trigger_sliding_wall(Feature::Instance & feature);
void trigger_portcullis(Feature::Instance & feature);

//-------------------------------------------------------------------------------------------------
// Module interface

void init()
{
	s_features.reserve(200);
	s_features_to_update.reserve(20);
}

void clear()
{
	s_features.clear();
	s_next_trigger_id = 0;
	s_features_to_update.clear();
}

void serialize(ISerializer& s)
{
	s.srz_vector(s_features, "Feature::s_features");
	s.srz_int(s_next_trigger_id);
	s.srz_vector(s_features_to_update, "Feature::s_features_to_update");
}

int get_new_trigger()
{
	++s_next_trigger_id;
	return s_next_trigger_id - 1;
}

void spawn(Vec3 pos, Terrain::Type type)
{
	if (Check(Terrain::is_feature(type)))
	{
		add_feature_internal(pos, type, c_Invalid, c_Invalid);

		// Feature-specific initialization.
		switch (type)
		{
			case Terrain::Chest:
				init_chest(s_features.back());
				break;
			case Terrain::Desk:
				init_desk(s_features.back());
				break;
			case Terrain::ShopSeed:
				register_for_updates(s_features.back());
				break;
			// special initialization
			case Terrain::Scanner:
			case Terrain::FlipendoButton:
			case Terrain::SlidingWall:
			case Terrain::Portcullis:
				DebugBreak("Spawn Feature with trigger");
				break;
			// no initialization needed
			// case Terrain::Armour:
			// case Terrain::TorchUnlit:  // cosmetic torch, can also spawn as trigger
			// case Terrain::TorchLit:
			// case Terrain::Portrait:
		}
	}
}

void spawn(Vec3 pos, Terrain::Type type, int trigger)
{
	if (Check(Terrain::is_feature(type)))
	{
		add_feature_internal(pos, type, c_Invalid, trigger);

		// Feature-specific initialization.
		switch (type)
		{
			case Terrain::Scanner:
				register_for_updates(s_features.back());
				break;
			case Terrain::TorchUnlit:  // can also spawn as cosmetic (no trigger)
			case Terrain::FlipendoButton:
			case Terrain::SlidingWall:
			case Terrain::Portcullis:
				break;
			default:
				DebugBreak("Spawn Feature without trigger");
				break;
		}
	}
}

void update_all()
{
	//
	//  The goal here is to have every feature update exactly once
	//    -> This is tricky because features might be added or removed during updates
	//    -> Including other features (e.g. #1 destroys #2 and adds #3)
	//      -> The affected feature might be later or earlier in the list
	//    -> If an earlier feature is removed, the current feature can be moved during its update
	//      -> Making references to it invalid, including its index
	//
	//  Keeping a list of feature indexes doesn't work
	//    -> Elements might get skipped if other elements are removed during updates
	//      -> e.g. if a trigger removes features
	//    -> Having features return whether they need to be removed isn't enough
	//      -> A button might remove a door, and I wouldn't know about the door
	//    -> I don't think anything could get updated twice, but you never know...
	//
	//  Other things that don't solve this:
	//    -> Going through the array backwards
	//      -> Feature #2 might remove feature #1, swapping in already-updated feature #3
	//      -> Feature #2 might remove feature #1, moving feature #2 during its update
	//    -> Iterating over the whole s_features array
	//      -> You still get all the same problems
	//
	//  Currently, we use positions as keys:
	//    -> Adding a position to this array registers the feature for updates
	//    -> If the feature is gone, we remove it instead of updating it
	//      -> Also if the feature has a non-updating type (it was replaced)
	//    -> We have to update the list whenever a feature's world position changes
	//      -> So ALWAYS use the Feature::move function
	//    -> Do not remove features from the s_features_to_update array except here
	//      -> Notably, not in the Feature::remove functions
	//    -> A feature that removes itself will still be in the array
	//      -> It will be removed next time through
	//      -> I'm declaring this Not A Problem.
	//    -> When we register a feature, we first check if its position is already registered
	//      -> If so, we do not register a second copy
	//      -> This happens if a registered feature is removed and
	//         another one is added in the same place during the same update cycle
	//        -> Notably if a registered feature replaces itself with a new registered feature
	//        -> Or between updates cycles (e.g. because of a spell activating a trigger)
	//
	//  A sparse array solves the problem of removing features, but not of adding them.
	//    -> The feature might be added before or after the current update spot
	//      -> Thus, it might get updated or not
	//  It might work if we always add features at the end of the sparse array
	//    -> Then we could do a separate pass to fill in the holes
	//    -> The indexes would not be stable
	//

	for (int i = 0; i < Util::Size(s_features_to_update); ++i)
	{
		if (update_at(s_features_to_update[i]) == false)
		{
			// feature has been removed
			Util::RemoveSwap(s_features_to_update, i);
			--i;
		}
	}
}

void move(Vec3 old_pos, Vec3 new_pos)
{
	int const index = find_feature(old_pos);
	if (index != c_Invalid)
	{
		Terrain::Type terrain = World::read().get_terrain(old_pos);
		World::edit().set_terrain(old_pos, Terrain::Open);
		World::edit().set_terrain(new_pos, terrain);

		s_features[index].pos = new_pos;

		// update s_features_to_update
		for (int i = 0; i < Util::Size(s_features_to_update); ++i)
		{
			// We must not move any features around in the array
			//  -> update_all might be running

			if(s_features_to_update[i] == old_pos)
			{
				s_features_to_update[i] = new_pos;
			}
		}
	}
}

void remove(Vec3 pos, Terrain::Type new_terrain_type)
{
	assert(!Terrain::is_feature(new_terrain_type));

	int const index = find_feature(pos);
	if (index != c_Invalid)
	{
		Util::RemoveSwap(s_features, index);
		World::edit().set_terrain(pos, new_terrain_type);

		// We do not change s_features_to_update here
		//  -> update_all might be running
		//  -> update_all will remove features from s_features_to_update as needed
	}
}

void remove(Vec3 pos)
{
	remove(pos, Terrain::Open);
}

//-------------------------------------------------------------------------------------------------
// Helper function implementations

int find_feature(Vec3 pos)
{
	return Util::FindIndexByKey(s_features, &Feature::Instance::pos, pos);
}

void add_feature_internal(Vec3 pos, Terrain::Type terrain, int hp, int payload)
{
	World::edit().set_terrain(pos, terrain);
	s_features.push_back({
		.pos = pos,
		.hp = hp,
		.payload = payload,
		});
}

void damage(Vec3 pos, Damage::Packet const& damage_packet)
{
	// This function invalidates many references to features, including indexes
	//  -> all warnings from damage_basic apply

	switch (World::read().get_terrain(pos))
	{
	case Terrain::Desk:
		damage_basic(pos, damage_packet, Material::Wood, "desk");  // invalidates references and indexes
		break;
	case Terrain::TorchUnlit:
		if (damage_packet.type == Damage::Type::Fire)
		{
			light_torch(pos);
		}
		break;
	}
}

//-------------------------------------------------------------------------------------------------
// Feature-specific functions

void register_for_updates(Feature::Instance& feature)
{
	for (int i = 0; i < Util::Size(s_features_to_update); i++)
	{
		if (s_features_to_update[i] == feature.pos)
		{
			return;  // already registered
		}
	}

	// otherwise add
	s_features_to_update.push_back(feature.pos);
}

void init_chest(Feature::Instance& feature)
{
	// Todo: Variable treasure based on current map

	Item::Handle top(c_Invalid);

	if (Random::coinflip())
	{
		Item::Handle new_potion = Item::make_potion_by_level(
			World::read().find_map_difficulty(feature.pos) + 1.0f);
		new_potion.stack_onto(top);
		top = new_potion;
	}

	int const num_beans = Random::in_range(3,6);
	for (int i = 0; i < num_beans; ++i)
	{
		Item::Handle new_bean = Item::make_bbb();
		new_bean.stack_onto(top);
		top = new_bean;
	}

	feature.payload = (int)top;
}

void open_chest(Vec3 pos)
{
	int const feature_index = find_feature(pos);
	if (Check(feature_index != c_Invalid))
	{
		Draw::pos_message(pos, "The chest bursts open!");

		Vec3TempList open_pos;
		open_pos.reserve(9);
		Pathfind::find_open_neighbours(pos, {}, open_pos);
		open_pos.push_back(pos);

		Feature::Instance& feature = s_features[feature_index];
		Item::Handle item_stack = (Item::Handle)feature.payload;
		while (item_stack.valid())
		{
			Item::Handle next_item = Item::unstack(item_stack);
			World::edit().add_item(Random::from_vector(open_pos), next_item);
		}

		Feature::remove(pos);  // feature becomes invalid
	}
}

void init_desk(Feature::Instance& feature)
{
	feature.hp = Random::in_range(3, 8);  // health
}

bool update_at(Vec3 pos)
{
	int const feature_index = find_feature(pos);
	if (feature_index == c_Invalid)
	{
		return false;  // nothing to update here
	}

	Feature::Instance& feature = s_features[feature_index];
	Terrain::Type feature_type = World::read().get_terrain(feature.pos);
	switch (feature_type)
	{
	// good features -- keep updating them
	case Terrain::Scanner:
		update_scanner(feature);
		return true;
	case Terrain::ShopSeed:
		update_shop_seed(feature);
		return true;

	default:
		// the feature here does not need updates
		//  -> there was probably a different feature here before, but it was replaced
		return false;
	}
}

void update_scanner(Feature::Instance& feature)
{
	if (Player::pos().z == feature.pos.z &&
		chessboard(Player::pos().xy(), feature.pos.xy()) < 5 &&
		World::read().is_visible(feature.pos))
	{
		// this Feature is removed when it triggers itself
		trigger_all(feature.payload);  // feature becomes invalid
	}
}

void update_shop_seed(Feature::Instance& feature)
{
	// TODO: Is there some way feature updates could work without visiblity?
	//  -> We have to recalculate the entire visibility state for them
	//  -> But we don't want things activating through walls

	if (Player::pos().z == feature.pos.z &&
		chessboard(Player::pos().xy(), feature.pos.xy()) < 5 &&
		World::read().is_visible(feature.pos))
	{
		Vec3 pos = feature.pos;
		Feature::remove(pos);  // feature becomes invalid

		// TODO: Spawn a real shop
		Draw::pos_message(pos, "A shop appears!");
		spawn(pos, Terrain::Chest);
	}
}

void damage_basic(Vec3 pos, Damage::Packet const& damage_packet,
                  Material const material, std::string const name)
{
	// This function invalidates many references to features, including indexes
	//  -> The index of the feature at pos becomes invalid, as do any higher indexes
	//  -> Lower indexes and references to those features are OK
	//  -> Features might move around in s_features
	//  -> s_features will not be reallocated
	// Copy any data you will need after calling this into local variables first
	// The feature ight not exist after calling this
	//  -> If it does, references and indexes are still valid

	int const feature_index = find_feature(pos);
	if (Check(feature_index != c_Invalid))
	{
		float const resistance =
			c_Resistances[(int)material][damage_packet.type];
		int const damage_adjusted = (int)(damage_packet.amount * resistance);

		Feature::Instance& feature = s_features[feature_index];
		feature.hp -= damage_adjusted;

		if (feature.hp <= 0)
		{
			Draw::pos_message(pos, "The " + name + " is destroyed!");
			Feature::remove(pos);  // feature becomes invalid
		}
		else if (damage_adjusted > 0)
		{
			switch (damage_packet.type)
			{
			case Damage::Basic:
				Draw::pos_message(pos, "The " + name + " is damaged.");
				break;
			case Damage::ToLife:
				Draw::pos_message(pos, "The " + name + " is damaged biologically.");
				DebugBreak("Feature should not be damaged by ToLife damage.");
				break;
			case Damage::Fire:
				Draw::pos_message(pos, "The " + name + " is burned.");
				break;
			case Damage::Acid:
				Draw::pos_message(pos, "The " + name + " is burned by acid.");
				break;
			}
		}
		else
		{
			Draw::pos_message(pos, "The " + name + " is not affected.");
		}
	}
}

void light_torch(Vec3 pos)
{
	int const feature_index = find_feature(pos);
	if (Check(feature_index != c_Invalid))
	{
		Draw::pos_message(pos, "The torch bursts into flames!");
		World::edit().set_terrain(pos, Terrain::TorchLit);

		int trigger = s_features[feature_index].payload;
		if (!is_any_unlit_torch_with_trigger(trigger))
		{
			trigger_all(trigger);  // feature becomes invalid
		}
	}
}

bool is_any_unlit_torch_with_trigger(int trigger)
{
	for (const Feature::Instance & feature : s_features)
	{
		if (feature.payload == trigger &&
			World::read().get_terrain(feature.pos) == Terrain::TorchUnlit)
		{
			return true;
		}
	}
	return false;
}

void open_portrait(Vec3 pos)
{
	int const feature_index = find_feature(pos);
	if (Check(feature_index != c_Invalid))
	{
		Draw::pos_message(pos, "The portrait swings open!");
		Feature::remove(pos);  // feature becomes invalid
	}
}

void activate_flipendo_button(Vec3 pos)
{
	int const feature_index = find_feature(pos);
	if (Check(feature_index != c_Invalid))
	{
		// only print a message for the one button we cast on
		//  -> other buttons on the same trigger flip silently
		Draw::pos_message(pos, "The button flips.");

		// this Feature is removed when it triggers itself
		int trigger = s_features[feature_index].payload;
		trigger_all(trigger);  // feature_index becomes invalid
	}
}

void trigger_all(int trigger)
{
	// This function invalidates all references to features, including indexes
	//  -> Features might move around in s_features
	//  -> s_features might be reallocated
	// Copy any data you will need after calling this into local variables first
	// If you need any feature after calling this, call find_feature again
	//  -> And remember that the feature might not exist anymore

	if (trigger == c_Invalid)
	{
		return;
	}

	// search backwards so indexes stay consistant
	for (int i = Util::Size(s_features) - 1; i >= 0; --i)
	{
		Feature::Instance& feature = s_features[i];
		if (feature.payload != trigger)
		{
			continue;
		}

		Terrain::Type feature_type = World::read().get_terrain(feature.pos);
		switch (feature_type)
		{
		case Terrain::Scanner:
		case Terrain::FlipendoButton:
			trigger_remove_yourself(feature);
			break;
		case Terrain::SlidingWall:
			trigger_sliding_wall(feature);
			break;
		case Terrain::Portcullis:
			trigger_portcullis(feature);
			break;
		}
	}
}

void trigger_remove_yourself(Feature::Instance & feature)
{
	Feature::remove(feature.pos, Terrain::Wall);  // feature becomes invalid
}

void trigger_sliding_wall(Feature::Instance & feature)
{
	Draw::pos_message(feature.pos, "A wall slides aside!");
	Feature::remove(feature.pos);  // feature becomes invalid
}

void trigger_portcullis(Feature::Instance & feature)
{
	Draw::pos_message(feature.pos, "A portcullis opens!");
	Feature::remove(feature.pos);  // feature becomes invalid
}

} // namespace Feature
