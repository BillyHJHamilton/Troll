#pragma once

#if _DEBUG

#include "MenuList.h"

// Debug cheat menu.
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
		ToggleRevealMap,
		SetLogCategories,
	};
};

// Used to turn logging categories on and off.
class MenuDebugLogCategories : public MenuList
{
public:
	void refresh();

	virtual void handle_input (int key);

protected:
	static int constexpr c_EnableAll = -2;
	static int constexpr c_DisableAll = -3;
};

#endif // _DEBUG
