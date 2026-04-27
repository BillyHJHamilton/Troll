#include "Feature.h"

#include "Damage.h"
#include "Debug.h"
#include "Draw.h"
#include "Grammar.h"
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
//  - DoorOpen - no payload
//  - DoorClosed - no payload
//  - DoorLocked - no payload
//  - DoorColloportus - payload is countdown until it reopens
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

using Itr = SparseVector<Feature::Instance>::Itr;

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

Itr find_feature(Vec3 pos);
Itr add_feature_internal(Vec3 pos, Terrain::Type terrain, int hp, int payload);
void remove_feature(Feature::Itr feature, Terrain::Type new_terrain_type);

void register_for_updates(Feature::Itr feature);
void init_chest(Itr feature);
void init_desk(Itr feature);

void update_feature(Itr feature);
void update_scanner(Itr feature);
void update_door_closed(Itr feature);
void update_door_colloportus(Itr feature);
void update_shop_seed(Itr feature);

void damage_basic(Vec3 pos, Damage::Packet const& damage_packet,
                  Material const material, std::string const name);

void light_torch(Vec3 pos);
bool is_any_unlit_torch_with_trigger(int trigger);

// Warning: Calling this normally causes features to be added/removed.
// Do not assume that feature references will remain valid afterwards.
void trigger_all(int trigger);

void trigger_sliding_wall(Itr feature);
void trigger_portcullis(Itr feature);

//-------------------------------------------------------------------------------------------------
// Module interface

void init()
{
	s_features.reserve(500);
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
		Itr new_feature = add_feature_internal(pos, type, c_Invalid, c_Invalid);

		// Feature-specific initialization.
		switch (type)
		{
			case Terrain::Chest:
				init_chest(new_feature);
				break;
			case Terrain::Desk:
				init_desk(new_feature);
				break;
			case Terrain::DoorClosed:
			case Terrain::ShopSeed:
				register_for_updates(new_feature);
				break;
			// special initialization
			case Terrain::Scanner:
			case Terrain::FlipendoButton:
			case Terrain::SlidingWall:
			case Terrain::Portcullis:
				DebugBreak("Spawn Feature with trigger");
				break;
			case Terrain::DoorColloportus:
				DebugBreak("Never spawn DoorColloportus directly");
				break;
			// no initialization needed
			// case Terrain::Armour:
			// case Terrain::TorchUnlit:  // cosmetic torch, can also spawn as trigger
			// case Terrain::TorchLit:
			// case Terrain::Portrait:
			// case Terrain::DoorOpen:
			// case Terrain::DoorLocked:
		}
	}
}

void spawn(Vec3 pos, Terrain::Type type, int trigger)
{
	if (Check(Terrain::is_feature(type)))
	{
		Feature::Itr new_feature = add_feature_internal(pos, type, c_Invalid, trigger);

		// Feature-specific initialization.
		switch (type)
		{
			case Terrain::Scanner:
				register_for_updates(new_feature);
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
	std::vector<Feature::Itr,Scratch<Feature::Itr>> to_update;
	to_update.reserve(s_features.size() / 10); // just a guess
	for (Feature::Itr feature = s_features.begin(); feature; ++feature)
	{
		if (feature->needs_update)
		{
			to_update.push_back(feature);
		}
	}

	for (Feature::Itr& feature : to_update)
	{
		if (feature.valid())
		{
			update_feature(feature);
		}
	}
}

void move(Vec3 old_pos, Vec3 new_pos)
{
	Feature::Itr feature = find_feature(old_pos);
	if (feature.valid())
	{
		Terrain::Type terrain = World::read().get_terrain(old_pos);
		World::edit().set_terrain(old_pos, Terrain::Open);
		World::edit().set_terrain(new_pos, terrain);

		feature->pos = new_pos;
	}
}

//-------------------------------------------------------------------------------------------------
// Helper function implementations

Feature::Itr find_feature(Vec3 pos)
{
	int const index = s_features.find_index_by_key(&Feature::Instance::pos, pos);
	return s_features.get_itr(index);
}

Feature::Itr add_feature_internal(Vec3 pos, Terrain::Type terrain, int hp, int payload)
{
	World::edit().set_terrain(pos, terrain);
	int new_index = s_features.insert({
		.pos = pos,
		.hp = hp,
		.payload = payload,
		});
	return s_features.get_itr(new_index);
}

void remove_feature(Feature::Itr feature, Terrain::Type new_terrain_type)
{
	World::edit().set_terrain(feature->pos, new_terrain_type);
	feature.remove_current();
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

void register_for_updates(Feature::Itr feature)
{
	feature->needs_update = true;
}

void init_chest(Feature::Itr feature)
{
	Item::Handle top(c_Invalid);

	// Slightly better than normal for this level.
	float const difficulty = World::read().find_map_difficulty(feature->pos) + 1.0f;
	Loot::stack(Loot::Chest_Main, top, Creature::None, difficulty);

	int const num_beans = Random::in_range(3,6);
	for (int i = 0; i < num_beans; ++i)
	{
		Loot::stack(Loot::Bean, top, Creature::None, difficulty);
	}

	feature->payload = (int)top;
}

void open_chest(Vec3 pos)
{
	Feature::Itr feature = find_feature(pos);
	if (Check(feature.valid()))
	{
		Draw::pos_message(pos, "The chest bursts open!");

		Vec3TempList open_pos;
		open_pos.reserve(9);
		Pathfind::find_open_neighbours(pos, {}, open_pos);
		open_pos.push_back(pos);

		Item::Handle item_stack = (Item::Handle)feature->payload;
		while (item_stack.valid())
		{
			Item::Handle next_item = Item::unstack(item_stack);
			World::edit().add_item(Random::from_vector(open_pos), next_item);
		}

		remove_feature(feature, Terrain::Open);
	}
}

void init_desk(Feature::Itr feature)
{
	feature->hp = Random::in_range(3, 8);
}

void update_feature(Feature::Itr feature)
{
	assert(feature.valid());
	Terrain::Type feature_type = World::read().get_terrain(feature->pos);
	switch (feature_type)
	{
		case Terrain::Scanner:
			update_scanner(feature);
			break;
		case Terrain::DoorClosed:
			update_door_closed(feature);
			break;
		case Terrain::DoorColloportus:
			update_door_colloportus(feature);
			break;
		case Terrain::ShopSeed:
			update_shop_seed(feature);
			break;
	}
}

void update_scanner(Feature::Itr feature)
{
	if (Player::pos().z == feature->pos.z &&
		World::read().has_los(feature->pos, Player::pos(), 5))
	{
		// This feature is removed when it triggers itself.
		trigger_all(feature->payload);
	}
}

void update_door_closed(Feature::Itr feature)
{
	Creature::Handle creature_on_door = Creature::creature_at_pos(feature->pos);
	if (creature_on_door != Creature::None)
	{
		Draw::creature_message(creature_on_door, std::format("{} {} the door.",
			Grammar::You(creature_on_door), Grammar::verbs("open", creature_on_door)));
		World::edit().set_terrain(feature->pos, Terrain::DoorOpen);
	}
}

void update_door_colloportus(Feature::Itr feature)
{
	--feature->payload;
	if (feature->payload <= 0)
	{
		Draw::pos_message(feature->pos, "A door unlocks.");
		World::edit().set_terrain(feature->pos, Terrain::DoorClosed);
	}
}

void update_shop_seed(Feature::Itr feature)
{
	if (Player::pos().z == feature->pos.z &&
		World::read().has_los(feature->pos, Player::pos(), 5) &&
		!Creature::creature_at_pos(feature->pos).valid())
	{
		Vec3 const pos = feature->pos;
		remove_feature(feature, Terrain::Open);

		Player::stop_automove();
		Draw::pos_message(pos, "A shop appears!");

		// TODO: Spawn a real shop
		spawn(pos, Terrain::Chest);
	}
}

void damage_basic(Vec3 pos, Damage::Packet const& damage_packet,
                  Material const material, std::string const name)
{
	Feature::Itr feature = find_feature(pos);
	if (Check(feature.valid()))
	{
		float const resistance =
			c_Resistances[(int)material][damage_packet.type];
		int const damage_adjusted = (int)(damage_packet.amount * resistance);

		feature->hp -= damage_adjusted;

		if (feature->hp <= 0)
		{
			Draw::pos_message(pos, "The " + name + " is destroyed!");
			remove_feature(feature, Terrain::Open);
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
	Feature::Itr feature = find_feature(pos);
	if (feature.valid())
	{
		Draw::pos_message(pos, "The torch bursts into flames!");
		World::edit().set_terrain(pos, Terrain::TorchLit);

		int trigger = feature->payload;
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
	Feature::Itr feature = find_feature(pos);
	if (Check(feature.valid()))
	{
		Draw::pos_message(pos, "The portrait swings open!");
		remove_feature(feature, Terrain::Open);
	}
}

void unlock_door(Vec3 pos)
{
	Feature::Itr feature = find_feature(pos);
	if (Check(feature.valid()))
	{
		Draw::pos_message(pos, "The door unlocks!");
		World::edit().set_terrain(pos, Terrain::DoorClosed);
		feature->needs_update = true;
	}
}

void lock_door(Vec3 pos)
{
	Feature::Itr feature = find_feature(pos);
	if (Check(feature.valid()))
	{
		Creature::Handle creature_on_door = Creature::creature_at_pos(feature->pos);
		if (creature_on_door != Creature::None)
		{
			Draw::creature_message(creature_on_door, std::format("The door is blocked by {}.",
				Grammar::you(creature_on_door)));
			return;
		}

		Terrain::Type old_type = World::read().get_terrain(pos);
		if (old_type == Terrain::DoorOpen)
		{
			Draw::pos_message(pos, "The door swings shut and locks!");
		}
		else
		{
			Draw::pos_message(pos, "The door locks!");
		}
		World::edit().set_terrain(pos, Terrain::DoorColloportus);
		feature->needs_update = true;
		feature->payload = Random::in_range(10, 15);
	}
}

void activate_flipendo_button(Vec3 pos)
{
	Feature::Itr feature = find_feature(pos);
	if (Check(feature.valid()))
	{
		// only print a message for the one button we cast on
		//  -> other buttons on the same trigger flip silently
		Draw::pos_message(pos, "The button flips.");

		// This feature is removed when it triggers itself.
		int const trigger = feature->payload;
		trigger_all(trigger);
	}
}

void trigger_all(int trigger)
{
	if (trigger == c_Invalid)
	{
		return;
	}

	for (Feature::Itr feature = s_features.begin(); feature; ++feature)
	{
		if (feature->payload != trigger)
		{
			continue;
		}

		Terrain::Type feature_type = World::read().get_terrain(feature->pos);
		switch (feature_type)
		{
		case Terrain::Scanner:
			remove_feature(feature, Terrain::Open);
			break;
		case Terrain::FlipendoButton:
			remove_feature(feature, Terrain::Wall);
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

void trigger_sliding_wall(Feature::Itr feature)
{
	Draw::pos_message(feature->pos, "A wall slides aside!");
	remove_feature(feature, Terrain::Open);
}

void trigger_portcullis(Feature::Itr feature)
{
	Draw::pos_message(feature->pos, "A portcullis opens!");
	remove_feature(feature, Terrain::Open);
}

} // namespace Feature
