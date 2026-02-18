#pragma once

#include "MenuList.h"
#include <functional>

class MenuLoad : public MenuList
{
public:
	virtual void handle_input (int key) override;
	void refresh();
};
