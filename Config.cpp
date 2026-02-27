#include "Config.h"

#include "SerializeSaveLoad.h"

#include "BearLibTerminal.h"
#include <filesystem>

namespace Config
{
//-------------------------------------------------------------------------------------------------
// Data

int constexpr c_ConfigVersionNumber = 0;

bool s_large_font = false;

//-------------------------------------------------------------------------------------------------
// Serialization

void serialize(ISerializer& s)
{
	int version_number = c_ConfigVersionNumber;
	s.srz_int(version_number);

	s.srz_bool(s_large_font);

	// TODO: When adding new config properties, could check min version number for each.
	// if (version_number >= 1) { ... }
}

//-------------------------------------------------------------------------------------------------
// Interface

void load_fonts (bool large)
{
	s_large_font = large;

	char const* cstr_FontPath = "FixedsysExcelsior302.ttf";
	char const* cstr_TilesPath = "Tiles.png";

	if (large)
	{
		// Great size for fullscreen on 1920x1080:
		terminal_setf("font: %s, size=16x32", cstr_FontPath);
		terminal_setf("tile font: %s, size=32x32, spacing=2x1", cstr_FontPath);
		terminal_setf("tile 0x3031: %s, size=16x16, spacing=2x1, "
			"resize=32x32, resize-filter=nearest", cstr_TilesPath);
	}
	else
	{
		terminal_setf("font: %s, size=8x16", cstr_FontPath);
		terminal_setf("tile font: %s, size=16x16, spacing=2x1", cstr_FontPath);

		// load custom glyphs
		terminal_setf("tile 0x3031: %s, size=16x16, spacing=2x1", cstr_TilesPath);
	}
}

void toggle_font_size ()
{
	load_fonts(!s_large_font);
}

int get_width()
{
	return terminal_state(TK_WIDTH);
}

int get_height()
{
	return terminal_state(TK_HEIGHT);
}

//-----------------------------------------------------------------------------
// Serialization

void save()
{
	if (!std::filesystem::exists("Save/"))
	{
		std::filesystem::create_directory("Save/");
	}

	SaveSerializer s("Save/Config.dat");
	Config::serialize(s);
}

void load()
{
	if (std::filesystem::exists("Save/Config.dat"))
	{
		LoadSerializer s("Save/Config.dat");
		Config::serialize(s);

		// And now apply the loaded settings.
		load_fonts(s_large_font);
	}
}

} // namespace Config
