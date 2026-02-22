#include "BuildWorld.h"

#include "Map.h"
#include "MapGenerator.h"
#include "Random.h"
#include "Terrain.h"
#include "World.h"

void BuildWorld()
{
	World& world = World::edit();

	//Box2 const map_box_0 = Box2(0, 0, 30, 30);
	//Box2 const map_box_e = Box2(30, 0, 50, 16);
	//Box2 const map_box_w = Box2(-30, 0, 30, 30);
	//Box2 const map_box_n = Box2(0, -30, 30, 30);
	//Box2 const map_box_s = Box2(0, 30, 20, 60);
	//
	//world.add_map(0, 0, map_box_0, Terrain::Wall);
	//world.add_map(0, 0, map_box_e, Terrain::Wall);
	//world.add_map(0, 0, map_box_w, Terrain::Wall);
	//world.add_map(0, 0, map_box_n, Terrain::Wall);
	//world.add_map(0, 0, map_box_s, Terrain::Wall);
	//
	//world.edit_map(0).get_generator().RequestConnection(1, 1);
	//world.edit_map(0).get_generator().RequestConnection(2, 2);
	//world.edit_map(0).get_generator().RequestConnection(3, 3);
	//world.edit_map(0).get_generator().RequestConnection(4, 2);
	//
	//world.edit_map(0).get_generator().Generate();
	//world.edit_map(1).get_generator().Generate();
	//world.edit_map(2).get_generator().Generate();
	//world.edit_map(3).get_generator().Generate();
	//world.edit_map(4).get_generator().Generate();

	Box2 const map_box1 = Box2(0, 0, 30, 30);

	// Create a stack of levels.
	int constexpr c_MaxZ = 6;

	// Pass 1 - allocate levels
	for (int z = 0; z <= c_MaxZ; ++z)
	{
		int const map_id = world.add_map(z, (z * 0.5f), map_box1, Terrain::Wall);
		assert(map_id == z);
	}
	//int const dungeon_id = world.add_map(-1, (4.0f), map_box1, Terrain::Wall);

	// Pass 2 - run generator
	for (int z = 0; z <= c_MaxZ; ++z)
	{
		MapGenerator& generator = world.edit_map(z).get_generator();

		MapGenerator::Parameters param{};
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
	}

	//world.edit_map(dungeon_id).get_generator().Generate();
}
