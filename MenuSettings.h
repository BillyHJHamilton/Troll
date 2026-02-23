#pragma once

#include "MenuList.h"

// Debug cheat menu.
class MenuSettings : public MenuList
{
public:
	void init();

	virtual void handle_input (int key);

protected:
	enum SettingsOption : byte
	{
		Back = 0,
		ToggleFontSize,
	};
};
