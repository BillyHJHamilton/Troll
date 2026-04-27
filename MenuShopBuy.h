#pragma once

#include "MenuList.h"

class MenuShopBuy : public MenuList
{
public:
	virtual void draw_screen();
	virtual Input::Result handle_input(int key);

	void refresh();

protected:
	void draw_selected_item();
	void try_buy_item();
};
