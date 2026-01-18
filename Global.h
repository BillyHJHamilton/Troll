#pragma once

struct Map;

void setup_global();

enum class GameMode : int
{
	Normal,
	Menu
};

extern bool g_quit_flag;
extern int g_tile_width_factor;
extern GameMode g_game_mode;
