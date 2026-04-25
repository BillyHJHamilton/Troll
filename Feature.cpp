#include "Feature.h"

#include "Damage.h"
#include "Debug.h"
#include "Draw.h"
#include "Item.h"
#include "Loot.h"
#include "Pathfind.h"
#include "Player.h"
#include "Random.h"
#include "Serialize.h"
#include "SparseVector.h"
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
	bool needs_update;

	// Parameters to be interpreted based on type of feature.
	int payload;
};
SparseVector<Feature::Instance> s_features;

int s_next_trigger_id = 0;

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
int add_feature_internal(Vec3 pos, Terrain::Type terrain, int hp, int payload);

void register_for_updates(Feature::Instance& feature);
void init_chest(Feature::Instance& feature);
void init_desk(Feature::Instance& feature);

void update_feature(int index);
void update_scanner(Feature::Instance& feature);
void update_shop_seed(Feature::Instance& feature);

void damage_basic(Vec3 pos, Damage::Packet const& damage_packet,
                  Material const material, std::string const name);

void light_torch(Vec3 pos);
bool is_any_unlit_torch_with_trigger(int trigger);

void trigger_all(int trigger);
void trigger_sliding_wall(Feature::Instance & feature);
void trigger_portcullis(Feature::Instance & feature);

//-------------------------------------------------------------------------------------------------
// Module interface

void init()
{
	s_features.reserve(200);
}

void clear()
{
	s_features.clear();
	s_next_trigger_id = 0;
}

void serialize(ISerializer& s)
{
	s_features.serialize(s);
	s.srz_int(s_next_trigger_id);
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
		int new_index = add_feature_internal(pos, type, c_Invalid, c_Invalid);

		// Feature-specific initialization.
		switch (type)
		{
			case Terrain::Chest:
				init_chest(s_features[new_index]);
				break;
			case Terrain::Desk:
				init_desk(s_features[new_index]);
				break;
			case Terrain::ShopSeed:
				register_for_updates(s_features[new_index]);
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
		int new_index = add_feature_internal(pos, type, c_Invalid, trigger);

		// Feature-specific initialization.
		switch (type)
		{
			case Terrain::Scanner:
				register_for_updates(s_features[new_index]);
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
	// First gather the list of features to update, then update them.
	// Any new features created during updating will not be updated till next turn.
	IntTempList to_update;
	to_update.reserve(s_features.size() / 10); // just a guess
	for (auto itr = s_features.begin(); itr; ++itr)
	{
		if (itr->needs_update)
		{
			to_update.push_back(itr.index());
		}
	}

	for (int const i : to_update)
	{
		// Check valid in case an earlier update destroyed one.
		if (s_features.is_valid(i))
		{
			update_feature(i);
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
	}
}

void remove(Vec3 pos, Terrain::Type new_terrain_type)
{
	assert(!Terrain::is_feature(new_terrain_type));

	int const index = find_feature(pos);
	if (index != c_Invalid)
	{
		s_features.remove(index);
		World::edit().set_terrain(pos, new_terrain_type);
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
	return s_features.find_index_by_key(&Feature::Instance::pos, pos);
}

int add_feature_internal(Vec3 pos, Terrain::Type terrain, int hp, int payload)
{
	World::edit().set_terrain(pos, terrain);
	return s_features.insert({
		.pos = pos,
		.hp = hp,
		.payload = payload,
		});
}

void damage(Vec3 pos, Damage::Packet const& damage_packet)
{
	switch (World::read().get_terrain(pos))
	{
		case Terrain::Desk:
			damage_basic(pos, damage_packet, Material::Wood, "desk");
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
	feature.needs_update = true;
}

void init_chest(Feature::Instance& feature)
{
	Item::Handle top(c_Invalid);

	// Slightly better than normal for this level.
	float const difficulty = World::read().find_map_difficulty(feature.pos) + 1.0f;
	Loot::stack(Loot::Chest_Main, top, Creature::None, difficulty);

	int const num_beans = Random::in_range(3,6);
	for (int i = 0; i < num_beans; ++i)
	{
		Loot::stack(Loot::Bean, top, Creature::None, difficulty);
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

		Feature::remove(pos);
	}
}

void init_desk(Feature::Instance& feature)
{
	feature.hp = Random::in_range(3, 8);
}

void update_feature(int index)
{
	assert(s_features.is_valid(index));
	Feature::Instance& feature = s_features[index];
	Terrain::Type feature_type = World::read().get_terrain(feature.pos);
	switch (feature_type)
	{
		case Terrain::Scanner:
			update_scanner(feature);
			break;

		case Terrain::ShopSeed:
			update_shop_seed(feature);
			break;
	}
}

void update_scanner(Feature::Instance& feature)
{
	if (Player::pos().z == feature.pos.z &&
		chessboard(Player::pos().xy(), feature.pos.xy()) < 5 &&
		World::read().is_visible(feature.pos))
	{
		// This feature is removed when it triggers itself.
		trigger_all(feature.payload);
	}
}

void update_shop_seed(Feature::Instance& feature)
{
	if (Player::pos().z == feature.pos.z &&
		chessboard(Player::pos().xy(), feature.pos.xy()) < 5 &&
		World::read().is_visible(feature.pos))
	{
		Vec3 const pos = feature.pos;
		Feature::remove(pos);

		// TODO: Spawn a real shop
		// TODO: Interrupt automove, etc.
		Draw::pos_message(pos, "A shop appears!");
		spawn(pos, Terrain::Chest);
	}
}

void damage_basic(Vec3 pos, Damage::Packet const& damage_packet,
                  Material const material, std::string const name)
{
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
			Feature::remove(pos);
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
			trigger_all(trigger);
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
		Feature::remove(pos);
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

		// This feature is removed when it triggers itself.
		int const trigger = s_features[feature_index].payload;
		trigger_all(trigger);
	}
}

void trigger_all(int trigger)
{
	if (trigger == c_Invalid)
	{
		return;
	}

	for (Feature::Instance& feature : s_features)
	{
		if (feature.payload != trigger)
		{
			continue;
		}

		Terrain::Type feature_type = World::read().get_terrain(feature.pos);
		switch (feature_type)
		{
		case Terrain::Scanner:
			Feature::remove(feature.pos, Terrain::Open);
			break;
		case Terrain::FlipendoButton:
			Feature::remove(feature.pos, Terrain::Wall);
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

void trigger_sliding_wall(Feature::Instance & feature)
{
	Draw::pos_message(feature.pos, "A wall slides aside!");
	Feature::remove(feature.pos);
}

void trigger_portcullis(Feature::Instance & feature)
{
	Draw::pos_message(feature.pos, "A portcullis opens!");
	Feature::remove(feature.pos);
}

} // namespace Feature
