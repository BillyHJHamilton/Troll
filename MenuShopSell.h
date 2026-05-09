#pragma once

#include "MenuList.h"

class MenuShopSell : public MenuList
{
public:
	virtual void draw_screen();
	virtual Input::Result handle_input(int key);
	virtual void on_close() override;

	void refresh();

protected:
	void draw_selected_item();
	void sell_item();
};
