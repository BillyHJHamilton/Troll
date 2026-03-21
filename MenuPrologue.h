#pragma once

#include "Menu.h"

class MenuPrologue : public IMenu
{
public:
	void refresh();

	virtual void draw_screen ();
	virtual Input::Result handle_input (int key);

protected:
	char const* m_text = nullptr;
};
