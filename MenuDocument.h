#pragma once

#include "Menu.h"
#include <string>
#include <functional>

// Static menu which shows text on the screen and does nothing else.
class MenuDocument : public IMenu
{
public:
	using VoidFunction = std::function<void()>;

	void init(std::string content);
	void init(std::string content, VoidFunction on_complete);

	virtual void draw_screen ();
	virtual void handle_input (int key);

protected:
	std::string m_text;
	VoidFunction m_on_complete;
};
