#include "BuildWorld.h"

#include "Creature.h"
#include "Map.h"
#include "MapGenerator.h"
#include "MapGridder.h"
#include "PerfTimer.h"
#include "Random.h"
#include "Terrain.h"
#include "World.h"

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
	//int const dungeon_id = world.add_map(-1, (4.0f), map_box1, Terrain::Wall);

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
		.min_chests = 5,
		.max_chests = 7,
	});

	for (int z = 0; z <= c_MaxZ; ++z)
	{
		Spawn::Parameters & param = world.edit_map(z).edit_spawn_param();

		param.trigger_weights[(int)(Spawn::TriggerType::None          )] = c_MaxZ;
		param.trigger_weights[(int)(Spawn::TriggerType::FlipendoButton)] = z;
		param.trigger_weights[(int)(Spawn::TriggerType::LightTorch    )] = z;

		// these are used depending on the trigger type
		param.door_weights[(int)(Spawn::DoorType::None       )] = c_MaxZ * 3 - z * 2;
		param.door_weights[(int)(Spawn::DoorType::Portrait   )] = c_MaxZ * 2;
		param.door_weights[(int)(Spawn::DoorType::SlidingWall)] = z;
		param.door_weights[(int)(Spawn::DoorType::Portcullis )] = z;

		param.percent_torches_lit = 80 - z * 10;
	}

	Spawn::post_world_setup();

	// Pass 2 - run generator
	for (int z = 0; z <= c_MaxZ; ++z)
	{
		MapGenerator& generator = world.edit_map(z).get_generator();

		MapGenerator::Parameters param{};
		//param.MinNumRooms += z;
		//param.MaxNumRooms += 2*z;
		generator.SetParameters(param);

		//if (z == 0)
		//{
		//	generator.RequestConnection(dungeon_id, 1);
		//}

		if (z < c_MaxZ)
		{
			generator.RequestConnection(z + 1, Random::in_range(2,3));
		}

		generator.Generate();
		MapGridder(world.edit_map(z), generator, z);
	}

	//world.edit_map(dungeon_id).get_generator().Generate();
}
