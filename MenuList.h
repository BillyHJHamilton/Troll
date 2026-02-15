#pragma once

#include "Menu.h"
#include <string>
#include <vector>

// Static menu which shows text on the screen and does nothing else.
class MenuList : public IMenu
{
public:
	struct Option
	{
		std::string label;
		int value = 0;
	};
	using OptionList = std::vector<Option>;

	virtual void draw_screen();
	virtual void handle_input(int key);

	void clear_list();

	void set_title(std::string title);
	void reserve(int n);
	void set_options(std::vector<Option>&& options);
	void add_option(std::string label, int value);

	void reset_cursor();
	void cursor_up();
	void cursor_down();
	void page_up();
	void page_down();
	void scroll_to_end();

	int get_cursor() const;
	const Option& get_selected() const;
	void remove_selected();

protected:
	void calc_layout();
	void calc_scroll_bottom();

	static int constexpr c_MaxLineY = 29;

	std::string m_title;
	OptionList m_options;
	int m_cursor = 0;

	int m_scroll_top = 0;
	int m_scroll_bottom = 0;

	// Layout parameters
	int m_list_start = 2;
	int m_max_lines = 27;
};
