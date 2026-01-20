#pragma once

void setup_global();

enum class GameMode : byte
{
	Normal,
	Menu
};

extern bool g_quit_flag;
extern int g_tile_width_factor;
extern int g_turn_number;
extern GameMode g_game_mode;
