#include "MenuMessages.h"
#include "Draw.h"

void MenuMessages::init()
{
	set_title("Message history:");

	// There's obviously better ways to do this, without copying everything.
	// We should store an index or iterator or something.
	// Maybe fix it after changing the message list to a circular array.
	// Also, it doesn't really make sense to use the list menu when you can't select or view anything.
	std::list<Draw::GameMessage> const& msg_list = Draw::read_messages();
	int i = 0;
	for (auto itr = msg_list.cbegin(); itr != msg_list.end(); ++itr)
	{
		add_option(itr->text, i++);
	}

	reset_cursor();
}
