#pragma once

namespace Config
{
	void load_fonts (bool large);

	bool large_font_enabled ();
	void toggle_font_size ();

	bool brighter_target_enabled ();
	void set_brighter_target (bool enabled);

	int get_width();
	int get_height();

	// File handling
	void save();
	void load();
}
