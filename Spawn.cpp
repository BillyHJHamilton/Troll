#include "Spawn.h"

#include "Creature.h"
#include "Debug.h"
#include "Feature.h"
#include "Game.h"
#include "Gingerbread.h"
#include "Map.h"
#include "Math.h"
#include "Pathfind.h"
#include "PerfTimer.h"
#include "Player.h"
#include "Random.h"
#include "Squad.h"
#include "Suggestion.h"
#include "Terrain.h"
#include "VectorUtil.h"
#include "World.h"

#include <format>

namespace Spawn
{

//-----------------------------------------------------------------------------
// Data

struct History
{
	int next_spawn_time = -1;
	int creatures_spawned = 0;
	int items_spawned = 0;
	int chests_spanwed = 0;

	bool has_ever_spawned() const { return next_spawn_time > -1; }
};

std::vector<Spawn::History> s_spawn_history;

// List of open positions on current level.
// Declared here in static memory to reduce allocations.
std::vector<Vec2> s_spawn_positions;

// Valid positions for treasure chests.  A subset of spawn positions.
std::vector<Vec2> s_special_positions;

//-----------------------------------------------------------------------------
// Helper declarations

// Caches a list of open spawn positions for the map.
// Remains valid until called against for another map, or until time passes.
void find_spawn_positions(const Map& map, int min_range_from_player);

// checks for open map, no item or creature, out of player's sight, far from player.
bool is_good_spawn_position(Map const & map, int min_range_from_player, Vec2 const & pos2);

// checks for Wall terrain, out of player's sight
bool is_good_wall_spawn_position(Map const & map, Vec2 const & pos2);

// Checks whether there are still cached spawn positions available.
// Call this before calling next_spawn_position().
bool has_spawn_positions();

// Removes and returns a random spawn position from the cached list.
Vec2 next_spawn_position();

void remove_spawn_position(Vec2 pos);

// Searches remaining spawn positions, so should be called after normal spawning.
// Calling next_chest_position removes from main list and chest list.
void find_chest_positions(const Map& map);
bool is_ok_chest_position(const Map& map, Vec2 pos);
bool has_special_positions();
Vec2 next_special_position();

// Check if a map meets the conditions to spawn.
bool is_map_ready(int map_id, int player_map);

// Do spawning for a single map.
void spawn_for_map(Map& map, History& history);

// Note: Must call find_spawn_positions first.
int spawn_boss(Map const& map);
int spawn_creatures(Map const& map, int creatures_to_spawn);
int spawn_items(Map const& map, int items_to_spawn);
int spawn_chests(Map& map, int chests_to_spawn);
int spawn_secret_areas(Map& map);
int spawn_secret_passages(Map& map);

bool requires_switch(Spawn::Door type);

// Decides on a creature or squad to spawn.
Spawn::Option choose_spawn_option(float target_difficulty);
void spawn_squad(int squad_id, Vec3 start_pos);

Vec3 choose_spawn_position(Map const & map, Suggestion::Type spawn_type);

//-----------------------------------------------------------------------------
// Interface

void clear()
{
	s_spawn_history.clear();
}

void serialize(ISerializer& s)
{
	s.srz_vector(s_spawn_history, "s_spawn_history");
}

void post_world_setup()
{
	int const num_maps = World::read().num_maps();
	Util::Fill(s_spawn_history, num_maps, Spawn::History{});
}

void check_spawning()
{
	int const player_map = World::read().find_map(Player::pos());
	if (!Util::IsValidIndex(s_spawn_history, player_map))
	{
		return;
	}

	for (int map_id = 0; map_id < World::read().num_maps(); ++map_id)
	{
		if (is_map_ready(map_id, player_map))
		{
			Map& map = World::edit().edit_map(map_id);
			History& history = s_spawn_history[map_id];

			if (Debug::enabled(Debug::Map))
			{
				std::cout << std::format("\nSpawning for map {} at difficulty level {}.\n",
					map_id, map.get_difficulty());
			}

			spawn_for_map(map, history);
		}
	}
}

bool difficulty_in_range (float difficulty, float target_difficulty)
{
	if (Math::FloatGreater(difficulty, target_difficulty + Spawn::c_MaxOverLevel) ||
		Math::FloatLess(difficulty, target_difficulty - Spawn::c_MaxUnderLevel))
	{
		return false;
	}

	return true;
}

float probability_factor (float difficulty, float target_difficulty)
{
	if (Math::FloatGreater(difficulty, target_difficulty))
	{
		float const difference = difficulty - target_difficulty;
		return pow(Spawn::c_OverLevelFactor, difference);
	}
	else if (Math::FloatLess(difficulty, target_difficulty))
	{
		float const difference = target_difficulty - difficulty;
		return pow(Spawn::c_UnderLevelFactor, difference);
	}
	return 1.0f;
}

//-----------------------------------------------------------------------------
// Helper Implementations

// Caches a list of open spawn positions for the map.
// Remains valid until called against for another map, or until time passes.
void find_spawn_positions(const Map& map, int min_range_from_player)
{
	s_spawn_positions.clear();

	for (BoxItr itr(map.get_box_minus_border(1)); itr; ++itr)
	{
		Vec2 const pos2 = *itr;
		if (is_good_spawn_position(map, min_range_from_player, pos2))
		{
			s_spawn_positions.push_back(pos2);
		}
	}

	if (Debug::enabled(Debug::Map))
	{
		std::cout << std::format("Found {} valid spawn positions.\n",
			Util::Size(s_spawn_positions));
	}
}

bool is_good_spawn_position(Map const & map, int min_range_from_player, Vec2 const & pos2)
{
	Vec3 const pos3 = pos2.xyz(map.get_z());

	if (!Terrain::is_open(map.get_terrain(pos2)))
	{
		return false;
	}

	if (map.has_item(pos2))
	{
		return false;
	}

	if (World::read().is_visible(pos3))
	{
		return false;
	}

	if (Player::pos().z == map.get_z() &&
		chessboard(Player::pos().xy(), pos2) < min_range_from_player)
	{
		return false;
	}

	if (Creature::creature_at_pos(pos3) != Creature::None)
	{
		return false;
	}

	return true;
}

bool is_good_wall_spawn_position(Map const & map, Vec2 const & pos2)
{
	Vec3 const pos3 = pos2.xyz(map.get_z());

	if (map.get_terrain(pos2) != Terrain::Wall)
	{
		return false;
	}

	if (World::read().is_visible(pos3))
	{
		return false;
	}

	return true;
}

bool has_spawn_positions()
{
	return !s_spawn_positions.empty();
}

Vec2 next_spawn_position()
{
	int const i = Random::index(s_spawn_positions);
	Vec2 const pos = s_spawn_positions.at(i);
	Util::RemoveSwap(s_spawn_positions, i);
	return pos;
}

void remove_spawn_position(Vec2 position)
{
	Util::RemoveSwapFirstMatchingItem(s_spawn_positions, position);
}

bool is_ok_chest_position(const Map& map, Vec2 pos)
{
	CompassDirection constexpr dirs[4] = {
		c_CompassEast, c_CompassNorth, c_CompassWest, c_CompassSouth };

	for (CompassDirection const dir : dirs)
	{
		CompassDirection const l1 = get_counterclockwise(dir);
		CompassDirection const l2 = get_counterclockwise_90(dir);
		CompassDirection const r1 = get_clockwise(dir);
		CompassDirection const r2 = get_clockwise_90(dir);

		Terrain::Type const t_front = map.get_terrain(pos + c_Compass[dir]);
		Terrain::Type const t_back = map.get_terrain(pos - c_Compass[dir]);
		Terrain::Type const t_l1 = map.get_terrain(pos + c_Compass[l1]);
		Terrain::Type const t_l2 = map.get_terrain(pos + c_Compass[l2]);
		Terrain::Type const t_r1 = map.get_terrain(pos + c_Compass[r1]);
		Terrain::Type const t_r2 = map.get_terrain(pos + c_Compass[r2]);

		// We want it against a wall, with open space in front.
		// And not blocking a hallway on either side.
		bool const front_ok = Terrain::is_open(t_front);
		bool const back_ok = t_back == Terrain::Wall;
		bool const left_ok = (t_l1 == Terrain::Wall || Terrain::is_open(t_l2)) && t_l1 == t_l2;
		bool const right_ok = (t_r1 == Terrain::Wall || Terrain::is_open(t_r2)) && t_r1 == t_r2;

		if (front_ok && back_ok && left_ok && right_ok)
		{
			return true;
		}
	}
	return false;
}

void find_chest_positions(const Map& map)
{
	s_special_positions.clear();

	// TODO: What if there aren't enough suggestions?
	//  -> could fall back on the old system
	//  -> currently, fewer chests spawn (handled in spawn_chests)
	//  -> should this go directly in the spawn_chests function?
	//    -> need a new position table after spawning features anyway
	for (Vec2 const & pos : map.read_suggestions().get(Suggestion::TreasureNormal))
	{
		s_special_positions.push_back(pos);
	}
}

bool has_special_positions()
{
	return !s_special_positions.empty();
}

Vec2 next_special_position()
{
	int const i = Random::index(s_special_positions);
	Vec2 const pos = s_special_positions.at(i);
	Util::RemoveSwap(s_special_positions, i);
	Util::RemoveSwapFirstMatchingItem(s_spawn_positions, pos);
	return pos;
}

bool is_map_ready(int map_id, int player_map)
{
	const Spawn::History& history = s_spawn_history[map_id];
	const Spawn::Parameters& param = World::read().read_map(map_id).read_spawn_param();

	// Spawning doesn't start for a map until visited by player.
	if (!history.has_ever_spawned())
	{
		return (map_id == player_map);
	}

	if (history.creatures_spawned >= param.max_creatures ||
		Game::get_turn_number() < history.next_spawn_time)
	{
		// Map is done spawning, or on cooldown.
		return false;
	}

	return true;
}

void spawn_for_map(Map& map, History& history)
{
	PerfTimer perf("spawn_for_map");

	Spawn::Parameters const& param = map.read_spawn_param();

	bool const is_first_spawn = !history.has_ever_spawned();

	int creatures_to_spawn = 1;
	int items_to_spawn = 0;
	int chests_to_spawn = 0;
	int min_range = param.min_range_from_player;

	if (is_first_spawn)
	{
		creatures_to_spawn = Random::in_range(param.min_creatures, param.max_creatures);
		items_to_spawn = Random::in_range(param.min_items, param.max_items);
		chests_to_spawn = Random::in_range(param.min_chests, param.max_chests);
		min_range = 2;
	}

	// all features must spawn first
	//  -> then (re)call find_spawn_positions
	//    -> currently not needed
	//  -> otherwise creatures and items spawn on features
	if (chests_to_spawn > 0)
	{
		find_chest_positions(map);
		history.chests_spanwed += spawn_chests(map, chests_to_spawn);
	}

	if (is_first_spawn)
	{
		spawn_secret_areas(map);
		spawn_secret_passages(map);
	}

	find_spawn_positions(map, min_range);

	if (is_first_spawn)
	{
		history.creatures_spawned += spawn_boss(map);
	}

	history.creatures_spawned += spawn_creatures(map, creatures_to_spawn);

	if (items_to_spawn > 0)
	{
		history.items_spawned += spawn_items(map, items_to_spawn);
	}

	// Set next cooldown time.
	int const cooldown = Random::in_range(param.cooldown_min, param.cooldown_max);
	history.next_spawn_time = Game::get_turn_number() + cooldown;

	if (Debug::enabled(Debug::Map))
	{
		std::cout << std::format("Have spawned {}/{} total.  Next spawn in {}.\n",
			history.creatures_spawned, param.lifetime_max_creatures, cooldown);
	}
}

int spawn_boss(Map const& map)
{
	Creature::Type const boss_type = map.read_spawn_param().boss;

	if (Creature::is_valid_type(boss_type) && has_spawn_positions())
	{
		Vec2 const pos = next_spawn_position();
		Vec3 const pos3 = pos.xyz(map.get_z());

		Creature::Handle creature = Creature::spawn_creature(boss_type, pos3);
		if (Debug::enabled(Debug::Map))
		{
			std::cout << std::format(" - Spawned boss {} at ({},{}) - diff {}.\n",
				creature.long_name(), creature.pos().x, creature.pos().y,
				Gingerbread::read(creature.type()).difficulty);
		}

		return 1;
	}

	return 0;
}

// Note: Must call find_spawn_positions first.
int spawn_creatures(Map const& map, int creatures_to_spawn)
{
	float const difficulty = map.get_difficulty();
	int creatures_spawned = 0;
	while (has_spawn_positions() && creatures_spawned < creatures_to_spawn)
	{
		//Vec2 const pos = next_spawn_position();
		//Vec3 const pos3 = pos.xyz(map.get_z());

		Spawn::Option option = choose_spawn_option(map.get_difficulty());

		if (option.type == Option::None)
		{
			break;
		}

		else if (option.type == Option::Creature)
		{
			Creature::Type const creature_type = (Creature::Type)option.index;
			assert(Creature::is_valid_type(creature_type));

			float const creature_difficulty = Gingerbread::read(creature_type).difficulty;
			Suggestion::Type const suggestion_type =
				Suggestion::get_enemy_type(difficulty, creature_difficulty);
			Vec3 const pos3 = choose_spawn_position(map, suggestion_type);

			Creature::Handle creature = Creature::spawn_creature(creature_type, pos3);
			if (Debug::enabled(Debug::Map))
			{
				std::cout << std::format(" - Spawned {} at ({},{}) - diff {}.\n",
					creature.long_name(), creature.pos().x, creature.pos().y,
					Gingerbread::read(creature.type()).difficulty);
			}

			++creatures_spawned;
		}

		else if (option.type == Option::Squad)
		{
			Vec3 const pos3 = choose_spawn_position(map, Suggestion::EnemyModerate);
			spawn_squad(option.index, pos3);
			++creatures_spawned; // That still only counts as one!
		}
	}

	if (Debug::enabled(Debug::Map))
	{
		std::cout << std::format("Spawned {}/{} creatures.\n",
			creatures_spawned, creatures_to_spawn);
	}

	return creatures_spawned;
}

// Must call find_spawn_positions first.
int spawn_items(Map const& map, int items_to_spawn)
{
	float const difficulty = map.get_difficulty();
	int items_spawned = 0;
	while (has_spawn_positions() && items_spawned < items_to_spawn)
	{
		Vec2 const pos = next_spawn_position();
		Vec3 const pos3 = pos.xyz(map.get_z());

		if (Random::one_in(14))
		{
			Item::spawn_potion_by_level(pos3, difficulty);
		}
		else
		{
			Item::spawn_bbb(pos3);
		}
		++items_spawned;
	}

	if (Debug::enabled(Debug::Map))
	{
		std::cout << std::format("Placed {}/{} items.\n",
			items_spawned, items_to_spawn);
	}

	return items_spawned;
}

int spawn_chests(Map& map, int chests_to_spawn)
{
	int spawned = 0;
	while (has_special_positions() && spawned < chests_to_spawn)
	{
		Vec2 const pos = next_special_position();
		Feature::spawn(pos.xyz(map.get_z()), Terrain::Chest);
		++spawned;
	}

	if (Debug::enabled(Debug::Map))
	{
		std::cout << std::format("Placed {}/{} chests.\n",
			spawned, chests_to_spawn);
	}

	return spawned;
}

int spawn_secret_areas(Map& map)
{
	// no min range from player

	Spawn::Parameters const& param = map.read_spawn_param();

	// pre-calculate shorter list of door types for when we can't place a switch
	assert(Util::Size(param.door_types) > 0);
	std::vector<Door> switchless_types;
	std::vector<int> switchless_weights;
	switchless_types.reserve(Util::Size(param.door_types));
	switchless_weights.reserve(Util::Size(param.door_types));
	for (int i = 0; i < Util::Size(param.door_types); ++i)
	{
		if (param.door_weights[i] <= 0)
		{
			continue;
		}
		if (!requires_switch(param.door_types[i]))
		{
			switchless_types  .push_back(param.door_types  [i]);
			switchless_weights.push_back(param.door_weights[i]);
		}
	}
	// if no switchless door types, use open hallways
	if (Util::Size(switchless_types) == 0)
	{
		switchless_types  .push_back(Door::None);
		switchless_weights.push_back(1);
	}

	auto const & suggestions_vec = map.read_suggestions().get_secret_areas();
	IntTempList index_list = Util::GetIndices(suggestions_vec);
	Random::shuffle_vector(index_list);

	int spawned = 0;
	for (int index : index_list)
	{
		Suggestion::SecretAreaInstance const & suggestion = suggestions_vec[index];

		if (!is_good_spawn_position(map, 0, suggestion.door))
		{
			continue;
		}

		// choose the type of door

		bool is_button_good = suggestion.has_button;
		if (is_button_good)
		{
			if (!is_good_wall_spawn_position(map, suggestion.button))
			{
				is_button_good = false;
			}
		}

		Door type = Door::None;
		if (is_button_good)
		{
			// all types are valid, use full list
			int const r = Random::weighted_index(param.door_weights);
			type = param.door_types.at(r);
		}
		else
		{
			// can't place switch, use shorter list
			int const r = Random::weighted_index(switchless_weights);
			type = switchless_types.at(r);
		}

		// finally add the door

		int map_z = map.get_z();
		switch(type)
		{
		case Door::Portrait:
			Feature::spawn(suggestion.door.xyz(map_z), Terrain::Portrait);
			++spawned;
			break;
		case Door::FlipendoSwitch:
			assert(is_button_good);
			Feature::spawn_flipendo_switch(suggestion.button.xyz(map_z),
			                               suggestion.door  .xyz(map_z));
			++spawned;
			break;
		}
	}

	if (Debug::enabled(Debug::Map))
	{
		std::cout << std::format("Hid {} of {} possible secret areas.\n",
			spawned, Util::Size(index_list));
	}

	return spawned;
}

// Similar to spawn_secret_areas, but there are 2 of everything
// It always closes off as many secret passages as possible (no maximum)
int spawn_secret_passages(Map& map)
{
	// no min range from player

	Spawn::Parameters const& param = map.read_spawn_param();

	// pre-calculate shorter list of door types without None
	//  -> for when we can and cannot place a switch
	std::vector<Door> switch_types;
	std::vector<int> switch_weights;
	std::vector<Door> switchless_types;
	std::vector<int> switchless_weights;
	switch_types.reserve(Util::Size(param.door_types));
	switch_weights.reserve(Util::Size(param.door_types));
	switchless_types.reserve(Util::Size(param.door_types));
	switchless_weights.reserve(Util::Size(param.door_types));
	for (int i = 0; i < Util::Size(param.door_types); ++i)
	{
		if (param.door_types[i] == Door::None)
		{
			continue;  // secret passages must be secret
		}
		if (param.door_weights[i] <= 0)
		{
			continue;
		}

		switch_types  .push_back(param.door_types  [i]);
		switch_weights.push_back(param.door_weights[i]);
		if (!requires_switch(param.door_types[i]))
		{
			switchless_types  .push_back(param.door_types  [i]);
			switchless_weights.push_back(param.door_weights[i]);
		}
	}
	// if no door types, use portraits
	if (Util::Size(switch_types) == 0)
	{
		switch_types  .push_back(Door::Portrait);
		switch_weights.push_back(1);
	}
	if (Util::Size(switchless_types) == 0)
	{
		switchless_types  .push_back(Door::Portrait);
		switchless_weights.push_back(1);
	}

	auto const & suggestions_vec = map.read_suggestions().get_secret_passages();
	IntTempList index_list = Util::GetIndices(suggestions_vec);
	Random::shuffle_vector(index_list);

	int spawned = 0;
	for (int index : index_list)
	{
		Suggestion::SecretPassageInstance const & suggestion = suggestions_vec[index];

		if (!is_good_spawn_position(map, 0, suggestion.door1) ||
		    !is_good_spawn_position(map, 0, suggestion.door2))
		{
			continue;
		}

		bool are_buttons_good = suggestion.has_buttons;
		if (are_buttons_good)
		{
			if (!is_good_wall_spawn_position(map, suggestion.button1) ||
			    !is_good_wall_spawn_position(map, suggestion.button2))
			{
				are_buttons_good = false;
			}
		}

		Door type = Door::None;
		if (are_buttons_good)
		{
			// can't place switch, use longer list
			int const r = Random::weighted_index(switch_weights);
			type = switch_types.at(r);
		}
		else
		{
			// can't place switch, use shorter list
			int const r = Random::weighted_index(switchless_weights);
			type = switchless_types.at(r);
		}

		// finally add the door

		int map_z = map.get_z();
		switch(type)
		{
		case Door::Portrait:
			// open separately
			Feature::spawn(suggestion.door1.xyz(map_z), Terrain::Portrait);
			Feature::spawn(suggestion.door2.xyz(map_z), Terrain::Portrait);
			++spawned;
			break;
		case Door::FlipendoSwitch:
			assert(are_buttons_good);
			Feature::spawn_flipendo_switch_pair(suggestion.button1.xyz(map_z),
			                                    suggestion.door1  .xyz(map_z),
			                                    suggestion.button2.xyz(map_z),
			                                    suggestion.door2  .xyz(map_z));
			++spawned;
			break;
		}
	}

	if (Debug::enabled(Debug::Map))
	{
		std::cout << std::format("Hid {} of {} secret passages.\n",
			spawned, Util::Size(index_list));
	}

	return spawned;
}

bool requires_switch(Spawn::Door type)
{
	switch (type)
	{
	case Spawn::Door::FlipendoSwitch:
		return true;
	default:
		return false;
	}
}

Spawn::Option choose_spawn_option(float target_difficulty)
{
	Spawn::OptionTempList options;
	FloatTempList weights;
	options.reserve(Creature::Count);
	weights.reserve(Creature::Count);

	Gingerbread::find_spawn_options(target_difficulty, options, weights);
	Squad::find_spawn_options(target_difficulty, options, weights);

	if (Util::Size(options) > 0)
	{
		return options.at(Random::weighted_index(weights));
	}
	else
	{
		return {Option::None, c_Invalid};
	}
}

void spawn_squad(int squad_id, Vec3 start_pos)
{
	if (Check(Squad::is_defined(squad_id)))
	{
		Squad::Definition const& squad = Squad::read_definition(squad_id);

		if (Debug::enabled(Debug::Map))
		{
			std::cout << std::format(" - Spawning squad {} at ({},{}) - diff {}.\n",
				squad.debug_name, start_pos.x, start_pos.y, squad.difficulty);
		}

		Creature::TypeTempList to_spawn;
		to_spawn.reserve(Squad::c_MaxSquadSize);

		int const squad_id = Squad::find_free_index();
		if (squad_id == c_Invalid)
		{
			DebugBreak("Max squads reached!  Spawning aborted.");
			return;
		}

		for (Squad::Member const& member : squad.members)
		{
			int const num_to_spawn = Random::in_range(member.min_num, member.max_num);
			assert(Creature::is_valid_type(member.type));
			to_spawn.insert(to_spawn.end(), num_to_spawn, member.type);
		}

		Vec3TempList spawn_positions;
		Pathfind::NearestOpenParam nearest_open_param
		{
			.max_cost = 5,
			.num_to_find = Util::Size(to_spawn),
			.allow_start = true,
			.allow_visible = false
		};

		Pathfind::find_nearest_open(start_pos, nearest_open_param, spawn_positions);

		for (int i = 0;
			i < Util::Size(to_spawn) && i < Util::Size(spawn_positions);
			++i)
		{
			Creature::Handle creature = Creature::spawn_creature(to_spawn[i], spawn_positions[i]);
			creature.set_squad(squad_id);
			remove_spawn_position(spawn_positions[i].xy());

			if (Debug::enabled(Debug::Map))
			{
				std::cout << std::format("  - Spawned {} at ({},{}) - diff {}.\n",
					creature.long_name(), creature.pos().x, creature.pos().y,
					Gingerbread::read(creature.type()).difficulty);
			}
		}
	}

}

// Note: Must call find_spawn_positions first.
Vec3 choose_spawn_position(Map const & map, Suggestion::Type spawn_type)
{
	// method 1: use a random map suggestion
	/*
	// TODO: need to validate position
	//   -> see find_spawn_positions
	// TODO: better way to pick a position at random
	//   Another option if you want to iterate the list in a random order is to use
	//   TempIntList index_list = Util::GetIndices(suggestion_list);
	//   Random::shuffle_vector(index_list);
	//   for (int index : index_list)
	// TODO: need handle non-initial spawn
	int count  = 0;
	int chosen = -1;
	for (int i = 0; i < map.get_suggestions().GetCount(spawn_type); ++i)
	{
		if (map.get_suggestions().getByType(spawn_type)[i].when == Suggestion::WhenToSpawn::Initial)
		{
			++count;
			if (Random::one_in(count))  // always happens first time
			{
				chosen = i;
			}
		}
	}
	if (chosen >= 0)
	{
		Vec2 const pos = map.get_suggestions().getByType(spawn_type)[chosen].position1;
		// TODO: need to remove suggestion
		// TODO: need to remove from spawn positions
		return pos.xyz(map.get_z());
	}
	*/
	// method 2: any valid spawn position
	Vec2 const pos = next_spawn_position();
	return pos.xyz(map.get_z());
}

} // namespace Spawn
