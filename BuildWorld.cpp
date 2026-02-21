#include "BuildWorld.h"

#include "Map.h"
#include "MapGenerator.h"
#include "Random.h"
#include "Terrain.h"
#include "World.h"

void BuildWorld()
{
	World& world = World::edit();

	Box2 const map1_box = Box2(0, 0, 30, 30);
	int const map1_id = world.add_map(0, 0.0f, map1_box, Terrain::Wall);
	MapGenerator& gen1 = world.edit_map(0).get_generator();
	gen1.Generate();

	// Now let's create a number of additional levels on top!
	int constexpr c_NumLevels = 6;
	for (int z = 1; z <= c_NumLevels; ++z)
	{
		int const map_id = world.add_map(z, (z * 0.5f), map1_box, Terrain::Wall);

		MapGenerator::Parameters param{};
		if (z == c_NumLevels)
		{
			param.UpStairsToAdd = 0;
		}
		else
		{
			param.UpStairsToAdd = Random::in_range(2,3);
		}

		MapGenerator& generator = world.edit_map(map_id).get_generator();
		generator.SetParameters(param);
		generator.AddConnectingStairsAsSeedRooms(world.read_map(map_id - 1));
		generator.Generate();

		// Post-process to remove failed stairs from previous level.
		// This is very inelegant (and leaves invalid rooms in the MapGenerator metadata)
		// but a better solution would be more difficult to implement.
		for (Stairs::Pair pair : generator.GetFailedStairs())
		{
			Vec2 other_end = pair.first + Stairs::relative_move(pair.second).xy();
			world.edit_map(map_id - 1).remove_stairs(other_end);
		}
	}
}
