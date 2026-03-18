#pragma once

#include "MenuList.h"

// Debug cheat menu.
class MenuSettings : public MenuList
{
public:
	void init();

	virtual void draw_screen();
	virtual Input::Result handle_input (int key) override;

	virtual bool is_toggle (int option) override;
	virtual bool get_toggle_value (int option) override;
	virtual void on_toggle (int option, bool new_value) override;

protected:
	enum SettingsOption : byte
	{
		Back = 0,
		LargeFont,
		BrighterTargetCursor,
	};

	static int constexpr c_MenuWidth = 40;

	static const char* const cstr_LargeFont;
	static const char* const cstr_BrighterTargetCursor;
};
