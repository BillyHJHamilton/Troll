#pragma once

#include "Menu.h"
#include <string>
#include <vector>

// A scrolling list menu with a cursor on the left.
class MenuList : public IMenu
{
public:
	struct Option
	{
		std::string label;
		int value = 0;
		const char* colour = nullptr;
	};
	using OptionList = std::vector<Option>;

	virtual void draw_screen();
	virtual void handle_input(int key);

	void clear_list();

	void set_title(std::string title);
	void reserve(int n);
	void set_options(std::vector<Option>&& options);
	void add_option(std::string label, int value, char const* colour = nullptr);

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
	int max_line_y();
	void calc_layout();
	void calc_scroll_bottom();
	void on_resize();

	static int constexpr c_Indent = 3;

	std::string m_title;
	OptionList m_options;
	int m_cursor = 0;

	int m_scroll_top = 0;
	int m_scroll_bottom = 0;

	// Layout parameters
	int m_list_start = 2;
	int m_max_lines = 27;
};
