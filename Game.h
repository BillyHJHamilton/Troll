#pragma once

#include "Types.h"
#include <string>

class World;

enum class GameMode : byte
{
	Normal,
	Menu,
	Confirm,
};

namespace Game
{
	int constexpr c_MajorVersion = 0; // Significant milestone for promotional purposes.
	int constexpr c_MinorVersion = 1; // Larger change which may break save compatibility.
	int constexpr c_PatchVersion = 1; // Minor change which should preserve save compatibility.

	struct VersionNumber
	{
		int major = c_MajorVersion;
		int minor = c_MinorVersion;
		int patch = c_PatchVersion;

		void serialize(ISerializer& s);
		bool can_load() const;
	};

	// Initialization is in several layers:
	void init();	// Runs only once, when the program starts.
	void clear();	// Runs at the start of each game, before the main menu.
	void setup();	// Runs at the end of character creation, to start the game.

	// Save or check the file type label string.
	bool serialize_file_type_label(ISerializer& s);

	// Serializes the entire game state.
	bool try_serialize_all(ISerializer& s);

	// Redraw the screen and process input.
	void update();

	// Clears everything and returns to the main menu.
	void reset();

	void save();
	void load(std::string filename);

	GameMode get_mode();
	void set_mode(GameMode mode);
	int get_turn_number();
}
