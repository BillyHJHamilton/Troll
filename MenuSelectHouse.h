#pragma once

#include "MenuList.h"

class MenuSelectHouse : public MenuList
{
public:
	MenuSelectHouse();

	virtual void draw_screen();
	virtual void handle_input (int key);

protected:
	void select_house();
};
