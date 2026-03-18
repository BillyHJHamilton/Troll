#include "MenuLoad.h"

#include "Colour.h"
#include "Debug.h"
#include "SerializeSaveLoad.h"
#include "VectorUtil.h"

#include "BearLibTerminal.h"
#include <filesystem>

void MenuLoad::refresh()
{
	clear_list();
	metadata_list.clear();
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

				FileMetadata metadata = get_metadata(stem);
				metadata_list.push_back(metadata);
				char const* colour = (metadata.valid && metadata.version.can_load()) ?
					nullptr : cstr_Grey;
				add_option(stem, Util::LastIndex(metadata_list), colour);
			}
		}
	}
	
	add_option("Cancel", c_Invalid);

	reset_cursor();
}

void MenuLoad::draw_screen()
{
	MenuList::draw_screen();

	int const file_index = get_selected().value;
	if (Util::IsValidIndex(metadata_list, file_index))
	{
		FileMetadata const& metadata = metadata_list[file_index];

		if (!metadata.valid)
		{
			terminal_print(50, 2, "Invalid format.  This file cannot be loaded.");
		}
		else if (!metadata.version.can_load())
		{
			terminal_print(50, 2, "Save version does not match.  This file cannot be loaded.");
			terminal_print(50, 4, std::format("Game was saved on version {}.{}.{}.",
				metadata.version.major, metadata.version.minor, metadata.version.patch).c_str());
			terminal_print(50, 5, std::format("Currently running version {}.{}.{}.",
				Game::c_MajorVersion, Game::c_MinorVersion, Game::c_PatchVersion).c_str());
		}
		else
		{
			terminal_print(50, 2, metadata.player_data.name.c_str());
			terminal_print(50, 4, std::format("Level {}", metadata.player_data.level).c_str());
			terminal_print(50, 5, std::format("Turn {}", metadata.turn_number).c_str());
		}
	}
}

Input::Result MenuLoad::handle_input (int key)
{
	if (key == TK_ENTER)
	{
		int const file_index = get_selected().value;
		if (file_index == c_Invalid)
		{
			Menu::back();
			return Input::Result::Handled;
		}
		else if (Util::IsValidIndex(metadata_list, file_index))
		{
			FileMetadata const& metadata = metadata_list[file_index];
			if (metadata.valid && metadata.version.can_load())
			{
				std::string const& stem = get_selected().label;
				std::string filename = std::format("Save/{}.sav", stem);

				Game::save();
				Game::load(filename);
				return Input::Result::Handled;
			}
		}
	}
	else if (key == TK_ESCAPE)
	{
		Menu::back();
		return Input::Result::Handled;
	}

	return MenuList::handle_input(key);
}

/*static*/ MenuLoad::FileMetadata MenuLoad::get_metadata(std::string const& stem)
{
	std::string const filename = std::format("Save/{}.sav", stem);
	LoadSerializer reader(filename);

	FileMetadata metadata {};
	metadata.valid = Game::serialize_file_type_label(reader);
	if (metadata.valid)
	{
		metadata.version.serialize(reader);
		if (metadata.version.can_load())
		{
			metadata.player_data.serialize(reader);
			reader.srz_int(metadata.turn_number);
		}
	}
	return metadata;
}
