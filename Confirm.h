#pragma once

#include "Input.h"
#include <functional>

// Used to block other input until user responds to a prompt.
// A callback can be triggered in response to a valid input.
namespace Confirm
{
	void clear();
	Input::Result handle_input(int key);

	void press_enter(std::function<void(void)> on_enter);
}
