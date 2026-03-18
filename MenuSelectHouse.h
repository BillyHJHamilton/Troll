#pragma once

#include "MenuList.h"

class MenuSelectHouse : public MenuList
{
public:
	void init();

	virtual void draw_screen();
	virtual Input::Result handle_input (int key);

protected:
	void select_house();
};
