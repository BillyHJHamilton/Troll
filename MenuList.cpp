#include "MenuList.h"
#include "VectorUtil.h"
#include "BearLibTerminal.h"

void MenuList::draw_screen ()
{
	terminal_font("");
	dimensions_t dim = terminal_print(0, 0, m_title.c_str());
	int list_start = dim.height + 1;

	for (int i = 0; i < Util::Size(m_options); ++i)
	{
		terminal_print(2, list_start + i, m_options[i].label.c_str());
	}

	// Show a cursor
	terminal_put(0, list_start + m_cursor, '>');

	/*if (s_list_details_func &&
		Check(Util::IsValidIndex(s_options, s_selection)))
	{
		s_list_details_func(s_options[s_selection]);
	}*/
}

void MenuList::handle_input (int key)
{
	switch (key)
	{
		case TK_UP:
		case TK_KP_8:
			cursor_up();
			break;

		case TK_DOWN:
		case TK_KP_2:
			cursor_down();
			break;

		case TK_ENTER:
		case TK_ESCAPE:
			Menu::close();
			break;
	}
}

void MenuList::clear_list()
{
	m_title.clear();
	m_options.clear();
	m_cursor = 0;
}

void MenuList::set_title(std::string title)
{
	m_title = title;
}

void MenuList::set_options(std::vector<Option>&& options)
{
	m_options = options;
}

void MenuList::reserve(int n)
{
	m_options.reserve(n);
}

void MenuList::add_option(std::string text, int value)
{
	m_options.push_back({text,value});
}

void MenuList::reset_cursor()
{
	m_cursor = 0;
}

void MenuList::cursor_up()
{
	if (m_cursor > 0)
	{
		--m_cursor;
	}
}

void MenuList::cursor_down()
{
	if (m_cursor < Util::LastIndex(m_options))
	{
		++m_cursor;
	}
}

int MenuList::get_cursor() const
{
	return m_cursor;
}

const MenuList::Option& MenuList::get_selected() const
{
	return m_options.at(m_cursor);
}

void MenuList::remove_selected()
{
	Util::RemoveAt(m_options, m_cursor);
	if (m_cursor > Util::LastIndex(m_options))
	{
		--m_cursor;
	}
}
