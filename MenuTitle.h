#pragma once

#include "MenuList.h"

// Main menu and title screen
class MenuTitle : public MenuList
{
public:
	void init();
	virtual void handle_input (int key);

protected:
	enum TitleMenuOption : byte
	{
		NewGame = 0,
		LoadGame,
		Settings,
#if _DEBUG
		SetLogging,
#endif
		Quit
	};

	static char const* cstr_TitleText;
};
