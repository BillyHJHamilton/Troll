#pragma once

struct Map;

void setup_global();

Map & g_map();

extern bool g_quit_flag;
extern int g_tile_width_factor;
