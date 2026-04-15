#include "Feature.h"

#include "Debug.h"
#include "Draw.h"
#include "Item.h"
#include "Map.h"
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
//  - Portrait - no variables
//  - FlipendoButton - affected is door position
//                   - also_activate is another button that should activate with it
//                     - these link in a circle

struct Instance
{
	Vec3 pos;

	// Parameters to be interpreted based on type of feature.
	int payload;
	Vec3 affected;
	Vec3 also_activate;
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
			// special initialization
			case Terrain::FlipendoButton:
				DebugBreak("Spawn with Feature::spawn_button_button");
				break;
			// no initialization needed
			// case Terrain::Portrait:
		}
	}
}

void spawn_flipendo_button(Vec3 button_pos, Vec3 door_pos)
{
	World::edit().set_terrain(button_pos, Terrain::FlipendoButton);
	s_features.push_back({ .pos = button_pos });

	// init button
	s_features.back().affected = door_pos;
	s_features.back().also_activate = button_pos;  // links to itself: circle of 1

	// add the closed door
	int map_index = World::read().find_map(door_pos);
	World::edit().edit_map(map_index).set_terrain(door_pos.xy(), Terrain::Wall);
}

void spawn_flipendo_button_pair(Vec3 button1_pos, Vec3 door1_pos,
                                Vec3 button2_pos, Vec3 door2_pos)
{
	// these buttons link to each other in a circle of 2
	//  -> whichever one is hit will activate the other one

	// first button
	World::edit().set_terrain(button1_pos, Terrain::FlipendoButton);
	s_features.push_back({ .pos = button1_pos });
	s_features.back().affected = door1_pos;
	s_features.back().also_activate = button2_pos;

	// second button
	World::edit().set_terrain(button2_pos, Terrain::FlipendoButton);
	s_features.push_back({ .pos = button2_pos });
	s_features.back().affected = door2_pos;
	s_features.back().also_activate = button1_pos;

	// add the closed doors
	int map_index1 = World::read().find_map(door1_pos);
	World::edit().edit_map(map_index1).set_terrain(door1_pos.xy(), Terrain::Wall);
	int map_index2 = World::read().find_map(door2_pos);
	World::edit().edit_map(map_index2).set_terrain(door2_pos.xy(), Terrain::Wall);
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
		Util::RemoveSwap(s_features, index);
		World::edit().set_terrain(pos, new_terrain_type);
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

void activate_flipendo_button(Vec3 pos)
{
	int const feature_index = find_feature(pos);
	if (Check(feature_index != c_Invalid))
	{
		Feature::Instance& feature = s_features[feature_index];

		bool is_button_visible = World::read().is_visible(pos);

		// remove the door if needed
		//  -> if the passage has length 1, the other switch might have removed it already
		bool is_door_visible = false;
		int map_index = World::read().find_map(feature.affected);
		if (World::read().read_map(map_index).get_terrain(feature.affected.xy()) == Terrain::Wall)
		{
			is_door_visible = World::read().is_visible(feature.affected);
			World::edit().edit_map(map_index).set_terrain(feature.affected.xy(), Terrain::Open);
		}

		if (is_button_visible && is_door_visible)
		{
			Draw::pos_message(pos, "The button flips and a nearby wall slides open!");
		}
		else if (is_button_visible)
		{
			Draw::pos_message(pos, "The button flips.  What else happened?");
		}
		else if (is_door_visible)
		{
			// could happen if the button is activated remotely
			Draw::pos_message(pos, "A nearby wall slides open!");
		}

		// remove this button and activate the next one (if any)
		//  -> they are linked in a circle, so we MUST remove first
		Vec3 also_pos = feature.also_activate;
		Feature::remove(pos, Terrain::Wall);

		if (find_feature(also_pos) != c_Invalid)  // is another button
		{
			activate_flipendo_button(also_pos);
		}
	}
}

} // namespace Feature
