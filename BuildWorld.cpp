#include "BuildWorld.h"

#include "Creature.h"
#include "Door.h"
#include "Map.h"
#include "MapGenerator.h"
#include "MapGridder.h"
#include "PerfTimer.h"
#include "Random.h"
#include "Terrain.h"
#include "World.h"

#include <cassert>

void BuildWorld()
{
	PerfTimer perf("BuildWorld");

	World& world = World::edit();

	// Create a stack of levels.
	int constexpr c_MaxZ = 7;

	// Pass 1 - allocate levels
	for (int z = 0; z <= c_MaxZ; ++z)
	{
		Box2 const map_box{0, 0, 30, 30};
		//Box2 const map_box{0 - z, 0 - z, 30 + 2*z, 30 + 2*z};
		int const map_id = world.add_map(z, (z * 0.5f), map_box, Terrain::Wall);
		assert(map_id == z);
	}

	// Set map names
	world.edit_map(0).set_name("Hogwarts - Ground Floor");
	world.edit_map(1).set_name("Hogwarts - First Floor");
	world.edit_map(2).set_name("Hogwarts - Second Floor");
	world.edit_map(3).set_name("Hogwarts - Third Floor");
	world.edit_map(4).set_name("Hogwarts - Fourth Floor");
	world.edit_map(5).set_name("Hogwarts - Fifth Floor");
	world.edit_map(6).set_name("Hogwarts - Sixth Floor");
	world.edit_map(7).set_name("Hogwarts - Seventh Floor");

	// set spawn parameters
	world.edit_map(7).set_spawn_param({
		.boss = Creature::MarySue,
		.min_creatures = 9,
		.max_creatures = 10,
		.cooldown_min = 60,
		.cooldown_max = 120,
		.lifetime_max_creatures = 30,
		.min_treasures = 5,
		.max_treasures = 7,
	});

	for (int z = 0; z <= c_MaxZ; ++z)
	{
		MapGenerator::Parameters & gen_param = world.edit_map(z).get_generator().EditParameters();
		//gen_param.MinNumRooms += z;
		//gen_param.MaxNumRooms += 2*z;
		gen_param.IsShopSeed = Random::coinflip();
		gen_param.percent_torches_lit        = 80 - z * 10;
		gen_param.percent_monster_on_trigger = 30 - z *  2;

		Door::Parameters & door_param = world.edit_map(z).edit_door_param();

		door_param.locked_genus_weights[(int)(Door::LockedGenus::None   )] = c_MaxZ * 2 - z;
		door_param.locked_genus_weights[(int)(Door::LockedGenus::Spell  )] = c_MaxZ;
		door_param.locked_genus_weights[(int)(Door::LockedGenus::Trigger)] = z;

		// if we chose a spell door
		door_param.spelled_weights[(int)(Door::Spelled::Portrait     )] = c_MaxZ;
		door_param.spelled_weights[(int)(Door::Spelled::AlohamoraDoor)] = c_MaxZ;
		door_param.spelled_weights[(int)(Door::Spelled::Ectoplasm    )] = z;

		// if we chose a trigger door
		door_param.triggered_weights[(int)(Door::Triggered::SlidingWall)] = 1;
		door_param.triggered_weights[(int)(Door::Triggered::Portcullis )] = 1;
		door_param.trigger_weights[(int)(Door::TriggerType::FlipendoButton)] = 2;
		door_param.trigger_weights[(int)(Door::TriggerType::LightTorch    )] = 1;

		door_param.unlocked_weights[(int)(Door::Unlocked::None  )] = 3;
		door_param.unlocked_weights[(int)(Door::Unlocked::Open  )] = 2;
		door_param.unlocked_weights[(int)(Door::Unlocked::Closed)] = 1;

		assert(door_param.are_weights_valid());

		Spawn::Parameters& spawn_param = world.edit_map(z).edit_spawn_param();
		spawn_param.treasure_holder_weights[(int)(Spawn::TreasureHolder::Chest    )] = c_MaxZ;
		spawn_param.treasure_holder_weights[(int)(Spawn::TreasureHolder::SlimePool)] = z;
		spawn_param.percent_monster_on_treasure = 45 - z *  3;
	}

	Spawn::post_world_setup();

	// Pass 2 - run generator
	for (int z = 0; z <= c_MaxZ; ++z)
	{
		MapGenerator& generator = world.edit_map(z).get_generator();

		if (z < c_MaxZ)
		{
			generator.RequestConnection(z + 1, Random::in_range(2,3));
		}

		generator.Generate();
		MapGridder(world.edit_map(z), generator, z);
	}
}
