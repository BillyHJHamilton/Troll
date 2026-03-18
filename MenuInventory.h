#pragma once

#include "MenuList.h"

class MenuInventory : public MenuList
{
public:
	virtual void draw_screen();
	virtual Input::Result handle_input(int key);

	void refresh();

protected:
	void draw_selected_item();
	void try_use_item();
	void try_discard_item();
};
