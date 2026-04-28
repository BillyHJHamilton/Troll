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
		MapGenerator::Parameters & param = world.edit_map(z).get_generator().EditParameters();

		//param.MinNumRooms += z;
		//param.MaxNumRooms += 2*z;

		param.IsShopSeed = Random::coinflip();

		param.door_genus_weights[(int)(LockedDoorGenus::None   )] = c_MaxZ * 2 - z;
		param.door_genus_weights[(int)(LockedDoorGenus::Spell  )] = c_MaxZ;
		param.door_genus_weights[(int)(LockedDoorGenus::Trigger)] = z;

		param.spell_door_weights[(int)(SpellDoorType::Portrait     )] = c_MaxZ;
		param.spell_door_weights[(int)(SpellDoorType::AlohamoraDoor)] = c_MaxZ;
		param.spell_door_weights[(int)(SpellDoorType::Ectoplasm    )] = z;

		param.trigger_door_weights[(int)(TriggerDoorType::SlidingWall)] = 1;
		param.trigger_door_weights[(int)(TriggerDoorType::Portcullis )] = 1;

		param.trigger_weights[(int)(TriggerType::FlipendoButton)] = 2;
		param.trigger_weights[(int)(TriggerType::LightTorch    )] = 1;

		param.unlocked_door_weights[(int)(UnlockedDoorType::None  )] = 3;
		param.unlocked_door_weights[(int)(UnlockedDoorType::Open  )] = 2;
		param.unlocked_door_weights[(int)(UnlockedDoorType::Closed)] = 1;

		param.percent_torches_lit = 80 - z * 10;
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
