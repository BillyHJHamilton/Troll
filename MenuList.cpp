#include "MenuList.h"

#include "Codepoint.h"
#include "Colour.h"
#include "Config.h"
#include "Debug.h"
#include "VectorUtil.h"

#include "BearLibTerminal.h"
#include <format>

void MenuList::draw_screen ()
{
	terminal_font("");
	terminal_print(0, 0, m_title.c_str());

	calc_layout();
	calc_scroll_bottom();

	bool const goes_off_top = m_scroll_top > 0;
	bool const goes_off_bottom = m_scroll_bottom < Util::LastIndex(m_options);

	if (goes_off_top)
	{
		terminal_print(c_Indent, m_list_start - 1, "...");
	}

	for (int i = m_scroll_top; i <= m_scroll_bottom; ++i)
	{
		int const line_y = m_list_start + (i - m_scroll_top);

		if (is_toggle(m_options[i].value))
		{
			// Special case for drawing toggles.
			bool const enabled = get_toggle_value(m_options[i].value);
			if (enabled)
			{
				terminal_print(c_Indent, line_y,
					std::format("[[ON]] {}",
					m_options[i].label.c_str()).c_str());
			}
			else
			{
				terminal_print(c_Indent, line_y,
					std::format("[color=lighter grey][[  ]] {}[/color]",
					m_options[i].label).c_str());
			}
		}
		else
		{
			// Normal list option, not a toggle.
			if (m_options[i].colour != nullptr)
			{
				terminal_print(c_Indent, line_y, std::format("[color={}]{}[/color]",
					m_options[i].colour, m_options[i].label).c_str());
			}
			else
			{
				terminal_print(c_Indent, line_y, m_options[i].label.c_str());
			}
		}
	}

	if (goes_off_bottom)
	{
		terminal_print(c_Indent, max_line_y() + 1, "...");
	}

	// Show a cursor
	terminal_put(1, m_list_start + m_cursor - m_scroll_top, Codepoint::HandRight);
}

Input::Result MenuList::handle_input (int key)
{
	switch (key)
	{
		case TK_UP:
		case TK_KP_8:
			cursor_up();
			return Input::Result::Handled;

		case TK_DOWN:
		case TK_KP_2:
			cursor_down();
			return Input::Result::Handled;

		case TK_LEFT:
		case TK_KP_4:
		case TK_PAGEUP:
			page_up();
			return Input::Result::Handled;

		case TK_RIGHT:
		case TK_KP_6:
		case TK_PAGEDOWN:
			page_down();
			return Input::Result::Handled;

		case TK_ENTER:
		{
			int const value = get_selected().value;
			if (is_toggle(value))
			{
				on_toggle(value, !get_toggle_value(value));
				return Input::Result::Handled;
			}
			return Input::Result::Skipped;
		}

		case TK_ESCAPE:
			Menu::back();
			return Input::Result::Handled;

		case TK_RESIZED:
			on_resize();
			return Input::Result::Handled;
	}

	return Input::Result::Skipped;
}

void MenuList::clear_list()
{
	m_title.clear();
	m_options.clear();
	m_cursor = 0;
	calc_layout();
}

void MenuList::set_title(std::string title)
{
	m_title = title;
	calc_layout();
}

void MenuList::reserve(int n)
{
	m_options.reserve(n);
}

void MenuList::set_options(std::vector<Option>&& options)
{
	m_options = options;
	calc_layout();
}

void MenuList::add_option(std::string text, int value, char const* colour)
{
	m_options.push_back({text,value,colour});
	calc_layout();
}

void MenuList::reset_cursor()
{
	m_cursor = 0;
	m_scroll_top = 0;
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

void MenuList::page_up()
{
	if (m_cursor > 0)
	{
		m_cursor = std::max(0, m_cursor - m_max_lines);

		if (m_cursor < m_scroll_top)
		{
			m_scroll_top = m_cursor;
		}
	}
}

void MenuList::page_down()
{
	if (m_cursor < Util::LastIndex(m_options))
	{
		m_cursor = std::min(Util::LastIndex(m_options), m_cursor + m_max_lines);

		if (m_cursor > m_max_lines)
		{
			m_scroll_top = m_cursor - m_max_lines;
		}
	}
}

void MenuList::scroll_to_end()
{
	m_cursor = Util::LastIndex(m_options);

	if (m_cursor >= m_max_lines)
	{
		m_scroll_top = m_cursor - m_max_lines;
	}
}

void MenuList::on_resize()
{
	calc_layout();
	calc_scroll_bottom();

	int const overflow = std::max(0, Util::Size(m_options) - m_max_lines);

	if (m_scroll_top > overflow)
	{
		m_scroll_top = overflow;
	}
	else if (m_cursor >= m_scroll_bottom)
	{
		m_scroll_top = m_cursor - m_max_lines;
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

int MenuList::max_line_y()
{
	return Config::get_height() - 2;
}

void MenuList::calc_layout()
{
	dimensions_t const dim = terminal_measure(m_title.c_str());
	if (dim.height == 0 && !m_title.empty())
	{
		DebugBreak("Cannot init menus before terminal is ready.  Defer initialization to Menu::init.");
	}

	m_list_start = dim.height + 1;
	m_max_lines = max_line_y() - m_list_start;
}

void MenuList::calc_scroll_bottom()
{
	m_scroll_bottom = std::min(
		m_scroll_top + m_max_lines,
		Util::LastIndex(m_options));
}

//-------------------------------------------------------------------------------------------------
// Optional toggle interface
//
//void MenuList::enable_all_toggles()
//{
//	for (Option& option : m_options)
//	{
//		if (is_toggle(option.value))
//		{
//			on_toggle(option.value, true);
//		}
//	}
//}
//
//void MenuList::disable_all_toggles()
//{
//	for (Option& option : m_options)
//	{
//		if (is_toggle(option.value))
//		{
//			on_toggle(option.value, false);
//		}
//	}
//}

