#include "MenuList.h"
#include "VectorUtil.h"
#include "BearLibTerminal.h"

void MenuList::draw_screen ()
{
	terminal_font("");
	dimensions_t dim = terminal_print(0, 0, m_title.c_str());
	int const list_start = dim.height + 1;

	int const max_lines = c_MaxLineY - list_start;
	m_scroll_bottom = std::min(m_scroll_top + max_lines, Util::LastIndex(m_options));

	bool const goes_off_top = m_scroll_top > 0;
	bool const goes_off_bottom = m_scroll_bottom < Util::LastIndex(m_options);

	if (goes_off_top)
	{
		terminal_print(2, list_start - 1, "...");
	}

	for (int i = m_scroll_top; i <= m_scroll_bottom; ++i)
	{
		int const line_y = list_start + (i - m_scroll_top);
		terminal_print(2, line_y, m_options[i].label.c_str());
	}

	if (goes_off_bottom)
	{
		terminal_print(2, c_MaxLineY + 1, "...");
	}

	// Show a cursor
	terminal_put(0, list_start + m_cursor - m_scroll_top, '>');

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

		if (m_cursor < m_scroll_top)
		{
			--m_scroll_top;
		}
	}
}

void MenuList::cursor_down()
{
	if (m_cursor < Util::LastIndex(m_options))
	{
		++m_cursor;

		if (m_cursor > m_scroll_bottom)
		{
			++m_scroll_top;
		}
	}
}

void MenuList::scroll_to_end()
{
	m_cursor = Util::LastIndex(m_options);

	dimensions_t dim = terminal_measure(m_title.c_str());
	int const list_start = dim.height + 1;
	int const max_lines = c_MaxLineY - list_start;

	if (m_cursor >= max_lines)
	{
		m_scroll_top = m_cursor - max_lines;
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
