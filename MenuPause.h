#pragma once

#include "MenuList.h"

// In-game pause menu
class MenuPause : public MenuList
{
public:
	MenuPause();

	virtual void handle_input (int key);

protected:
	enum PauseMenuOption : byte
	{
		Resume = 0,
		SpellsKnown,
		Inventory,
		Help,
		MessageHistory,
		//Save,
		//Load,
		SaveAndQuit
	};
};
