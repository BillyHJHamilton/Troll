#pragma once

#include "MenuList.h"

// Menu which shows a list of spells.
// Description of the selected spell is shown on the right.
class MenuSpells : public MenuList
{
public:
	virtual void draw_screen ();
	virtual Input::Result handle_input (int key);

	// Show the menu with a list of spells known by the player.
	void show_known_spells ();

	// Show the menu with a list of possible starting spells.
	void show_starting_spells ();

protected:
	void draw_selected_spell ();
	void select_starting_spell ();

	enum class Mode
	{
		KnownSpells,
		StartingSpells
	};
	Mode m_mode = Mode::KnownSpells;

	int m_num_selected = 0;
};
