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
		DefeatAllEnemies,
		SetLogCategories,
	};
};

// Used to turn logging categories on and off.
class MenuDebugLogCategories : public MenuList
{
public:
	void init();

	virtual void handle_input (int key);

	virtual bool is_toggle(int option_value);
	virtual bool get_toggle_value(int option_value);
	virtual void on_toggle(int option_value, bool new_value);

protected:
	static int constexpr c_EnableAll = -2;
	static int constexpr c_DisableAll = -3;
};

#endif // _DEBUG
