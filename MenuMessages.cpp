#include "MenuMessages.h"
#include "Draw.h"

void MenuMessages::init()
{
	set_title("Message history:");

	// There's obviously better ways to do this, without copying everything.
	// We should store an index or iterator or something.
	// Maybe fix it after changing the message list to a circular array.
	// Also, it doesn't really make sense to use the list menu when you can't select or view anything.
	int const num_msg = Draw::get_num_recent_messages();
	for (int i = num_msg - 1; i >= 0; --i)
	{
		add_option(Draw::get_recent_message(i).text, i);
	}

	reset_cursor();
}
