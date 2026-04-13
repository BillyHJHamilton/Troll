#include "Feature.h"

#include "Debug.h"
#include "Draw.h"
#include "Item.h"
#include "Pathfind.h"
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
//  - Portrait - no payload

struct Instance
{
	Vec3 pos;

	// Parameters to be interpreted based on type of feature.
	int payload;
};
std::vector<Feature::Instance> s_features;

//-------------------------------------------------------------------------------------------------
// Helper declarations

int find_feature(Vec3 pos);

void init_chest(Feature::Instance& feature);

//-------------------------------------------------------------------------------------------------
// Module interface

void init()
{
	s_features.reserve(30);
}

void clear()
{
	s_features.clear();
}

void serialize(ISerializer& s)
{
	s.srz_vector(s_features, "Feature::s_chests");
}

bool is_at(Vec3 pos)
{
	return find_feature(pos) != c_Invalid;
}

void spawn(Vec3 pos, Terrain::Type type)
{
	if (Check(Terrain::is_feature(type)))
	{
		World::edit().set_terrain(pos, type);
		s_features.push_back({ .pos = pos });

		// Feature-specific initialization.
		switch (type)
		{
			case Terrain::Chest:
				init_chest(s_features.back());
				break;
			// no initialization needed:
			// case Terrain::Portrait:
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

void remove(Vec3 pos)
{
	int const index = find_feature(pos);
	if (index != c_Invalid)
	{
		Util::RemoveSwap(s_features, index);
		World::edit().set_terrain(pos, Terrain::Open);
	}
}

//-------------------------------------------------------------------------------------------------
// Helper function implementations

int find_feature(Vec3 pos)
{
	return Util::FindIndexByKey(s_features, &Feature::Instance::pos, pos);
}

//-------------------------------------------------------------------------------------------------
// Feature-specific functions

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

		Feature::remove(pos);
	}
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

} // namespace Feature
