#pragma once

#include "MenuList.h"

// Debug cheat menu
class MenuDebug : public MenuList
{
public:
	void init();

	virtual void handle_input (int key);

protected:
	enum DebugMenuOption : byte
	{
		Cancel = 0,
		LearnAllSpells,
		IncreaseStats,
	};
};
