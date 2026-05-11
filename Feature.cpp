#include "Feature.h"

#include "Damage.h"
#include "Debug.h"
#include "Draw.h"
#include "Grammar.h"
#include "Item.h"
#include "Loot.h"
#include "Math.h"
#include "Pathfind.h"
#include "Player.h"
#include "Random.h"
#include "Serialize.h"
#include "Shop.h"
#include "SparseVector.h"
#include "Spawn.h"
#include "Squad.h"
#include "Terrain.h"
#include "VectorUtil.h"
#include "World.h"

namespace Feature
{

//-------------------------------------------------------------------------------------------------
// Data

// Feature Type payloads:
//  - Chest - Item::Handle of item to add to map when chest is opened
//  - DoorColloportus - countdown until the door reopens
//  - TriggerOnCoutdown - countdown until the rtrigger triggers
//  - TriggerOnMonsterDead - Creature::Handle to trigger on the death of

struct Instance
{
	Vec3 pos;
	int hp;
	int trigger;
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

char const* cstr_TriggerFailed = "You hear a clunk.";

//-------------------------------------------------------------------------------------------------
// Helper declarations

Itr find_feature(Vec3 pos);
Itr add_feature_internal(Vec3 pos, Terrain::Type terrain, int hp, int trigger, int payload);
void remove_feature(Feature::Itr feature, Terrain::Type new_terrain_type);

void register_for_updates(Feature::Itr feature);
void init_chest(Itr feature);
void init_desk(Itr feature);

void update_feature(Itr feature);
void update_pressure_plate(Itr feature);
void update_tripwire(Feature::Itr feature, Axis check_axis);
void update_door_closed(Itr feature);
void update_door_colloportus(Itr feature);
void update_trigger_delay(Itr feature);
void update_trigger_on_monster_dead(Itr feature);
void update_shop_seed(Itr feature);

void damage_basic(Vec3 pos, Damage::Packet const& damage_packet,
                  Material const material, std::string const name);

void light_torch(Vec3 pos);
int count_features_of_a_type_with_trigger(Terrain::Type terrain, int trigger);

// Warning: Calling this normally causes features to be added/removed.
// Do not assume that feature references will remain valid afterwards.
void trigger_all(int trigger);

void trigger_sliding_wall(Itr feature);
void trigger_portcullis(Itr feature);
void trigger_portcullis_trap(Itr feature);
void trigger_monster_trap(Itr feature, bool is_retrigger_on_defeat);

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
		//                                                hp         trigger    payload
		Itr new_feature = add_feature_internal(pos, type, c_Invalid, c_Invalid, c_Invalid);

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
			case Terrain::PressurePlate:
			case Terrain::TripwireX:
			case Terrain::TripwireY:
			case Terrain::FlipendoButton:
			case Terrain::SlidingWall:
			case Terrain::Portcullis:
			case Terrain::PortcullisTrap:
			case Terrain::MonsterTrap:
			case Terrain::MonsterTrapAmbush:
				// TODO: Is there some better way to check this?
				//   -> And the other half in the spawn-with-trigger variant
				// TODO: Can we say which feature?
				DebugBreak("Spawn Feature with trigger");
				break;
			case Terrain::DoorColloportus:
				DebugBreak("Never spawn DoorColloportus directly");
				break;
			case Terrain::TriggerDelay:
				// TODO: Can these be collapsed into 1 case that says which feature?
				DebugBreak("Never spawn TriggerDelay directly");
				break;
			case Terrain::TriggerOnMonsterDead:
				DebugBreak("Never spawn TriggerOnMonsterDead directly");
				break;
			// no initialization needed - TODO: Do we need these?
			// case Terrain::Armour:
			// case Terrain::TorchUnlit:  // cosmetic torch, can also spawn as trigger
			// case Terrain::TorchLit:
			// case Terrain::Portrait:
			// case Terrain::Ectoplasm:
			// case Terrain::DoorOpen:
			// case Terrain::DoorLocked:
		}
	}
}

void spawn(Vec3 pos, Terrain::Type type, int trigger)
{
	if (Check(Terrain::is_feature(type)))
	{
		//                                                         hp         trigger  payload
		Feature::Itr new_feature = add_feature_internal(pos, type, c_Invalid, trigger, c_Invalid);

		// Feature-specific initialization.
		switch (type)
		{
			case Terrain::PressurePlate:
			case Terrain::TripwireX:
			case Terrain::TripwireY:
				register_for_updates(new_feature);
				break;
			case Terrain::TorchUnlit:  // can also spawn as cosmetic (no trigger)
			case Terrain::FlipendoButton:
			case Terrain::SlidingWall:
			case Terrain::Portcullis:
			case Terrain::PortcullisTrap:
			case Terrain::MonsterTrap:
			case Terrain::MonsterTrapAmbush:
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

Feature::Itr add_feature_internal(Vec3 pos, Terrain::Type terrain,
                                  int hp, int trigger, int payload)
{
	World::edit().set_terrain(pos, terrain);
	int new_index = s_features.insert({
		.pos = pos,
		.hp = hp,
		.trigger = trigger,
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
	Loot::stack(Loot::Treasure, top, Creature::None, difficulty);

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
		case Terrain::PressurePlate:
			update_pressure_plate(feature);
			break;
		case Terrain::TripwireX:
			update_tripwire(feature, c_AxisX);
			break;
		case Terrain::TripwireY:
			update_tripwire(feature, c_AxisY);
			break;
		case Terrain::DoorClosed:
			update_door_closed(feature);
			break;
		case Terrain::DoorColloportus:
			update_door_colloportus(feature);
			break;
		case Terrain::TriggerDelay:
			update_trigger_delay(feature);
			break;
		case Terrain::TriggerOnMonsterDead:
			update_trigger_on_monster_dead(feature);
			break;
		case Terrain::ShopSeed:
			update_shop_seed(feature);
			break;
	}
}

void update_pressure_plate(Feature::Itr feature)
{
	if (Player::pos() == feature->pos)
	{
		Draw::pos_message(feature->pos, "You step on a pressure plate.");

		// This feature is removed when it triggers itself.
		trigger_all(feature->trigger);
	}
}

void update_tripwire(Feature::Itr feature, Axis check_axis)
{
	assert(check_axis == c_AxisX || check_axis == c_AxisY);

	Axis other_axis = get_other_axis(check_axis);
	if (Player::pos().z == feature->pos.z &&
	    Player::pos()[other_axis] == feature->pos[other_axis])
	{
		// Check if we have a clear path to the player.
		//  -> we don't know which way we will look (+1 or -1)
		//  -> we know the tripwire cell is clear
		//  -> we have to check the player cell
		//    -> otherwise, it can trigger while the player is under the portcullis
		int delta = Math::Sign(feature->pos[check_axis] - Player::pos()[check_axis]);

		bool is_open = true;
		for (Vec3 pos = Player::pos();
		     pos != feature->pos;
		     pos[check_axis] += delta)
		{
			if (!Terrain::can_spawn(World::read().get_terrain(pos)))
			{
				is_open = false;
				break;  // found an obstruction, so stop looking
			}
		}

		if (is_open)
		{
			// This feature is removed when it triggers itself.
			trigger_all(feature->trigger);
		}
	}
}

void update_door_closed(Feature::Itr feature)
{
	// TODO: Opening the door should take a move
	//  -> It will have to be detected when a creatue is moving
	//    -> Including the player
	//  -> The movement AI will have to understand it
	//    -> Moving onto a door is a move, but does not consume a path step
	//    -> The move has not failed, even though the creature didn't move

	Creature::Handle creature_on_door = Creature::creature_at_pos(feature->pos);
	if (creature_on_door != Creature::None)
	{
		Draw::creature_message(creature_on_door, std::format("{} {} a door.",
			Grammar::You(creature_on_door), Grammar::verbs("open", creature_on_door)));
		World::edit().set_terrain(feature->pos, Terrain::DoorOpen);
	}
}

void update_door_colloportus(Feature::Itr feature)
{
	// payload is timer until it unlocks itself
	--feature->payload;
	if (feature->payload <= 0)
	{
		Draw::pos_message(feature->pos, "A door unlocks.");
		World::edit().set_terrain(feature->pos, Terrain::DoorClosed);
	}
}

void update_trigger_delay(Feature::Itr feature)
{
	// payload is timer until it triggers
	--feature->payload;
	if (feature->payload <= 0)
	{
		// This feature is removed when it triggers itself.
		trigger_all(feature->trigger);
	}
}

void update_trigger_on_monster_dead(Feature::Itr feature)
{
	// payload is handle to creature
	Creature::Handle creature = (Creature::Handle)feature->payload;
	if (!creature.valid())
	{
		int trigger = feature->trigger;
		int feature_count = count_features_of_a_type_with_trigger(
			Terrain::TriggerOnMonsterDead, trigger);
		if (feature_count <= 1)
		{
			// This feature is removed when it triggers itself.
			trigger_all(trigger);
		}
		else
		{
			remove_feature(feature, Terrain::Open);
		}
	}
}

void update_shop_seed(Feature::Itr feature)
{
	if (Player::pos().z == feature->pos.z &&
		World::read().has_los(feature->pos, Player::pos(), 5) &&
		!Creature::creature_at_pos(feature->pos).valid())
	{
		bool const success = Shop::try_spawn(feature->pos);
		if (success)
		{
			Vec3 const pos = feature->pos;
			remove_feature(feature, Terrain::Open);

			Player::stop_automove();
		}
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

		int trigger = feature->trigger;
		if (count_features_of_a_type_with_trigger(Terrain::TorchUnlit, trigger) == 0)
		{
			trigger_all(trigger);
		}
	}
}

int count_features_of_a_type_with_trigger(Terrain::Type terrain, int trigger)
{
	int count = 0;
	for (const Feature::Instance & feature : s_features)
	{
		if (feature.trigger == trigger &&
			World::read().get_terrain(feature.pos) == terrain)
		{
			++count;
		}
	}
	return count;
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

void clear_ectoplasm(Vec3 pos)
{
	Feature::Itr feature = find_feature(pos);
	if (Check(feature.valid()))
	{
		Draw::pos_message(pos, "The ectoplasm is scrubbed away!");
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
		feature->payload = Random::in_range(10, 15);  // timer until it unlocks itself
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
		trigger_all(feature->trigger);
	}
}

void trigger_all(int trigger)
{
	if (trigger == c_Invalid)
	{
		return;
	}

	// First gather the list of features to trigger, then trigger them.
	// Any new features created during triggering will not be triggered themselves.
	std::vector<Feature::Itr,Scratch<Feature::Itr>> to_trigger;
	to_trigger.reserve(s_features.size() / 10); // just a guess
	for (Feature::Itr feature = s_features.begin(); feature; ++feature)
	{
		if (feature->trigger == trigger)
		{
			to_trigger.push_back(feature);
		}
	}

	for (Feature::Itr& feature : to_trigger)
	{
		if (!feature.valid())
		{
			continue;
		}
		if (feature->trigger != trigger)
		{
			continue;
		}

		Terrain::Type feature_type = World::read().get_terrain(feature->pos);
		switch (feature_type)
		{
		case Terrain::PressurePlate:
		case Terrain::TripwireX:
		case Terrain::TripwireY:
		case Terrain::TriggerDelay:
		case Terrain::TriggerOnMonsterDead:
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
		case Terrain::PortcullisTrap:
			trigger_portcullis_trap(feature);
			break;
		case Terrain::MonsterTrap:
			trigger_monster_trap(feature, false);
			break;
		case Terrain::MonsterTrapAmbush:
			trigger_monster_trap(feature, true);
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

void trigger_portcullis_trap(Feature::Itr feature)
{
	if (Creature::creature_at_pos(feature->pos).valid())
	{
		Draw::pos_message(feature->pos, cstr_TriggerFailed);
	}
	else
	{
		Draw::pos_message(feature->pos, "A portcullis slams down!");
		World::edit().set_terrain(feature->pos, Terrain::Portcullis);
	}
}

void trigger_monster_trap(Feature::Itr feature, bool is_retrigger_on_defeat)
{
	int constexpr c_AmbushLockedTime = 100;

	float const difficulty = World::read().find_map_difficulty(feature->pos);
	Spawn::Option option = Spawn::choose_spawn_option(difficulty, Creature::Habitat::Trap);

	Creature::HandleList creature_list;
	creature_list.reserve(Squad::c_MaxSquadSize);

	if (option.type == Spawn::Option::Creature)
	{
		Vec3TempList spawn_positions;
		Pathfind::NearestOpenParam nearest_open_param
		{
			.max_cost = 3,
			.num_to_find = 1,
			.allow_start = true,
			.allow_visible = true,
		};
		Pathfind::find_nearest_open(feature->pos, nearest_open_param, spawn_positions);

		if (!spawn_positions.empty())
		{
			Vec3 spawn_pos = spawn_positions[0];
			Creature::Type const creature_type = (Creature::Type)option.index;
			assert(Creature::is_valid_type(creature_type));
			Creature::Handle creature = Creature::spawn_creature(creature_type, spawn_pos);
			creature_list.push_back(creature);

			Draw::pos_message(spawn_pos, std::format("A {} {} in.", creature.long_name(),
				Grammar::verbs("drop", creature)), creature.colour());

			if (Debug::enabled(Debug::Map))
			{
				std::cout << std::format("Trap spawned {} at ({},{}).\n",
					creature.long_name(), spawn_pos.x, spawn_pos.y);
			}
		}
	}
	else
	{
		if (Debug::enabled(Debug::Map))
		{
			std::cout << std::format("Trap is trying to spawn a squad at ({},{}).\n",
				feature->pos.x, feature->pos.y);
			// more will print in Spawn::spawn_squad
		}

		// drop as close to desired cell as possible
		//  -> they can land around the player
		int squad_index = Spawn::spawn_squad(option.index, feature->pos, true);
		if (squad_index != c_Invalid)
		{
			creature_list = Squad::get_squad(squad_index);
			assert(!creature_list.empty());

			// TODO: Squad names?
			Draw::pos_message(feature->pos, "Beasts drop in.", Squad::colour(option.index));
		}
	}

	if (creature_list.empty())
	{
		Draw::pos_message(feature->pos, cstr_TriggerFailed);

		if (is_retrigger_on_defeat)
		{
			// Oh, no!  We are locked in a room with no enemies!
			// The door will only open when we defeat the final one of zero total.
			// That's impossible, so how will we ever get out?

			World::edit().set_terrain(feature->pos, Terrain::TriggerDelay);
			feature->payload = 1;
			register_for_updates(feature);

			if (Debug::enabled(Debug::Map))
			{
				std::cout << std::format("Trap at ({},{}) failed to spawn monsters.\n",
					feature->pos.x, feature->pos.y);
			}

			return;
		}
	}

	if (is_retrigger_on_defeat)
	{
		// Replace this trap with a delayed trigger
		//  -> It will eventually activate the trigger again

		World::edit().set_terrain(feature->pos, Terrain::TriggerDelay);
		feature->payload = c_AmbushLockedTime;
		register_for_updates(feature);

		// Add 1 trigger to watch each creature spawned.
		//  -> When they are all dead, the trigger activates
		//  -> it doesn't matter where; they don't use their position
		// We cannot use Pathfind::find_nearest_open
		//  -> It will select terrain with features and they will get stomped
		// TODO: If we have triggers without features, these should be that
		//  -> We just want one per creature spawned
		//  -> Or one for all the spawned creatures, if the datastructure supports that

		int map_z = feature->pos.z;
		int creature_index = 0;
		Box2 check_area = Box2::around_tile(feature->pos.xy(), 5);
		std::cout << std::format("Placing triggers in box ({} - {}, {} - {})\n",
			check_area.min.x, check_area.inner_max().x,
			check_area.min.y, check_area.inner_max().y);
		for (Vec2 const& pos : check_area)
		{
			Vec3 pos3 = pos.xyz(map_z);
			if (World::read().get_terrain(pos3) == Terrain::Open)
			{
				std::cout << std::format("  Adding trigger at ({}, {})\tFeatures was at ({}, {})\n",
					pos.x, pos.y, feature->pos.x, feature->pos.y);
				Feature::Itr new_feature = add_feature_internal(
					pos3, Terrain::TriggerOnMonsterDead,
					c_Invalid, feature->trigger, (int)(creature_list[creature_index]));
				//  hp         trigger           payload

				register_for_updates(new_feature);

				++creature_index;
				if (creature_index >= Util::Size(creature_list))
				{
					// placed enough triggers
					break;
				}
			}
			else
			{
				std::cout << std::format("  Pos ({}, {}) was not open\n",
					pos.x, pos.y);
			}
		}
	}
	else
	{
		remove_feature(feature, Terrain::Open);
	}
}

} // namespace Feature
