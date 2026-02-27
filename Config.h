#pragma once

namespace Config
{
	void load_fonts (bool large);

	void toggle_font_size ();

	int get_width();
	int get_height();

	// File handling
	void save();
	void load();
}
