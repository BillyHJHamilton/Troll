#include "Global.h"

#include "Creature.h"
#include "Map.h"
#include "Player.h"

bool g_quit_flag;
bool g_game_over;
int g_turn_number;
GameMode g_game_mode;

void setup_global()
{
	g_quit_flag = false;
	g_turn_number = 0;
	g_game_mode = GameMode::Normal;

	Map & map = g_map();
	Box map_box = make_box(0, 0, 100, 100);
	map.init(map_box, Terrain::Wall);
	map.fill_box(make_box(4,4,15,15), Terrain::Open);
	map.fill_box(make_box(7,5,4,1), Terrain::Wall);
	map.clear_visibility();

	spawn_creature(Creature::Player, {4,4});
	spawn_creature(Creature::Neville_0, {6,9});
	spawn_creature(Creature::ColinCreevy_0, {14,15});
//	spawn_creature(Creature::ColinCreevy_0, {14,17});
//	spawn_creature(Creature::ColinCreevy_0, {16,15});
//	spawn_creature(Creature::ColinCreevy_0, {15,16});
//	spawn_creature(Creature::ColinCreevy_0, {8,13});
}
