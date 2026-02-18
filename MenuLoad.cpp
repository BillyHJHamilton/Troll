#include "MenuLoad.h"

#include "Debug.h"
#include "Game.h"
#include "BearLibTerminal.h"

#include <filesystem>

void MenuLoad::refresh()
{
	clear_list();
	set_title("Select a saved game to load:");

	if (std::filesystem::exists("Save/"))
	{
		for (const std::filesystem::directory_entry& entry :
			std::filesystem::directory_iterator("Save/"))
		{
			if (entry.exists()
				&& entry.is_regular_file()
				&& entry.path().has_extension()
				&& entry.path().extension().string() == ".sav")
			{
				std::string const stem = entry.path().stem().string();
				add_option(stem, 0);
			}
		}
	}
	
	add_option("Cancel", c_Invalid);

	reset_cursor();
}

void MenuLoad::handle_input (int key)
{
	if (key == TK_ENTER)
	{
		if (get_selected().value == c_Invalid)
		{
			Menu::show_title();
		}
		else
		{
			std::string const& stem = get_selected().label;
			std::string filename = std::format("Save/{}.sav", stem);

			Game::save();
			Game::load(filename);
		}
	}
	else if (key == TK_ESCAPE)
	{
		Menu::show_title();
	}
	else
	{
		MenuList::handle_input(key);
	}
}
