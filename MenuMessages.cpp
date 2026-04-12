#include "MenuMessages.h"
#include "Draw.h"

void MenuMessages::init()
{
	clear_list();
	set_title("Message history:");

	// There's obviously better ways to do this, without copying everything.
	// We should store an index or iterator or something.
	// Maybe fix it after changing the message list to a circular array.
	// Also, it doesn't really make sense to use the list menu when you can't select or view anything.
	int const num_msg = Draw::get_num_recent_messages();
	for (int i = num_msg - 1; i >= 0; --i)
	{
		Draw::GameMessage const& message = Draw::get_recent_message(i);
		add_option(message.text, i, message.colour);
	}

	reset_cursor();
}
