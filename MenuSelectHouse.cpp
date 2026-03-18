#include "MenuSelectHouse.h"

#include "Debug.h"
#include "Gingerbread.h"
#include "House.h"

#include "BearLibTerminal.h"

void MenuSelectHouse::init()
{
	set_title("What is your Hogwarts House?");

	m_options =
	{
		{House::name(House::Hufflepuff), House::Hufflepuff},
		{House::name(House::Ravenclaw), House::Ravenclaw},
		{House::name(House::Gryffindor), House::Gryffindor},
		{House::name(House::Slytherin), House::Slytherin}
	};
}

void MenuSelectHouse::draw_screen()
{
	MenuList::draw_screen();

	House::Type h = (House::Type)get_selected().value;
	if (House::is_valid(h))
	{
		dimensions_t dim = terminal_print(50, 2, House::name(h));
		terminal_print_ext(50, 4, 50, 20, 1, House::description(h));
	}
}

Input::Result MenuSelectHouse::handle_input (int key)
{
	switch (key)
	{
		case TK_ENTER:
			select_house();
			return Input::Result::Handled;

		case TK_ESCAPE:
			Menu::show_name_entry();
			return Input::Result::Handled;

		default:
			return MenuList::handle_input(key);
	}
}

void MenuSelectHouse::select_house()
{
	House::Type house = (House::Type)get_selected().value;
	assert(House::is_valid(house));
	Gingerbread::reset_player_stats(house);
	Menu::show_starting_spells();
}
