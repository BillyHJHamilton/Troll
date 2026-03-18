#pragma once

#include "Menu.h"
#include "Types.h"
#include <string>

class MenuName : public IMenu
{
public:
	virtual void draw_screen () override;
	virtual Input::Result handle_input (int key) override;

	void init();

protected:
	bool is_valid_character (char c);

	std::string name;
};

