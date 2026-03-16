#pragma once

#include "Menu.h"
#include <string>
#include <functional>

class MenuGameOver : public IMenu
{
public:
	void refresh();

	virtual void draw_screen ();
	virtual void handle_input (int key);

protected:
	std::string m_text;
};
