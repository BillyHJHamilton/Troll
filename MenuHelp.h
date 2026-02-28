#pragma once

#include "MenuList.h"

// Menu which shows a series of instruction pages.
class MenuHelp : public MenuList
{
public:
	virtual void draw_screen ();

	void init();
	void refresh();

protected:
	enum Page : byte
	{
		Movement,
		CastingSpells,
		Items,
		OtherCommands,
	};

	static int constexpr c_MenuWidth = 30;

	static const char* const cstr_Movement;
	static const char* const cstr_CastingSpells;
	static const char* const cstr_Items;
	static const char* const cstr_OtherCommands;
};
