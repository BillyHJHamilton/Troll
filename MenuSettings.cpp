#include "MenuSettings.h"

#include "Config.h"
#include "Debug.h"

#include "BearLibTerminal.h"

//-------------------------------------------------------------------------------------------------
// Main debug menu

const char* const MenuSettings::cstr_LargeFont =
	"Doubles the size of the font (and game window).  "
	"The enlarged window requires a screen resolution of at least 1920x1080.";

const char* const MenuSettings::cstr_BrighterTargetCursor =
	"Changes the targeting cursor to a purple colour (instead of dark grey).  "
	"This makes it easier to see with a low display brightness.\n\n"
	"However, it may be harder to read the colours of highlighted creatures.";

void MenuSettings::init()
{
	set_title("Game Settings:");
	set_options(
	{
		{"Back", SettingsOption::Back},
		{"Large Font", SettingsOption::LargeFont},
		{"Brighter Target Cursor", SettingsOption::BrighterTargetCursor},
	});
}

void MenuSettings::draw_screen()
{
	MenuList::draw_screen();

	char const* description = nullptr;
	switch ((SettingsOption)get_selected().value)	
	{
		case SettingsOption::LargeFont:
			description = cstr_LargeFont;
			break;
		case SettingsOption::BrighterTargetCursor:
			description = cstr_BrighterTargetCursor;
			break;
	}

	if (description)
	{
		int const width = 70;
		dimensions_t dim = terminal_print(c_MenuWidth, 2, get_selected().label.c_str());
		int const height = Config::get_height() - (3 + dim.height);
		terminal_print_ext(c_MenuWidth, 3 + dim.height, width, height, 1, description);
	}
}

Input::Result MenuSettings::handle_input (int key)
{
	if (key == TK_ENTER && 
		get_selected().value == SettingsOption::Back)
	{
		Config::save();
		Menu::back();
		return Input::Result::Handled;
	}

	return MenuList::handle_input(key);
}

bool MenuSettings::is_toggle (int option)
{
	return option > (int)SettingsOption::Back;
}

bool MenuSettings::get_toggle_value (int option)
{
	switch ((SettingsOption)option)
	{
		case LargeFont:
			return Config::large_font_enabled();
		case BrighterTargetCursor:
			return Config::brighter_target_enabled();

		default:
			DebugBreak();
			return false;
	}

}

void MenuSettings::on_toggle (int option, bool new_value)
{
	switch ((SettingsOption)option)
	{
		case LargeFont:
			Config::toggle_font_size();
			break;

		case BrighterTargetCursor:
			Config::set_brighter_target(new_value);
			break;

		default:
			DebugBreak();
	}
}

