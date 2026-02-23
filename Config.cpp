#include "Config.h"
#include "BearLibTerminal.h"

namespace Config
{
	bool s_large_font = false;

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
}