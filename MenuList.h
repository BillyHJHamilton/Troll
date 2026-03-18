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

	virtual void draw_screen() override;
	virtual Input::Result handle_input(int key) override;

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

	//---------------------------------------------------------------------------------------------
	// Optional toggle interface

	// These functions can be used to add toggles to a menu.
	// Create your list of options as usual.  Then define the three functions below to
	// indicate which of the menu options are toggles, and what they do.
	// See MenuSettings for an example.

	virtual bool is_toggle(int option_value) { return false; }
	virtual bool get_toggle_value(int option_value) { return false; }
	virtual void on_toggle(int option_value, bool new_value) {}

	// Not sure if these are useful
	//void enable_all_toggles();
	//void disable_all_toggles();
};
