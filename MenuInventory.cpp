#include "MenuInventory.h"

#include "Action.h"
#include "Debug.h"
#include "Inventory.h"
#include "VectorUtil.h"

#include "BearLibTerminal.h"
#include <format>

void MenuInventory::draw_screen()
{
	MenuList::draw_screen();

	draw_selected_item();
}

void MenuInventory::handle_input (int key)
{
	switch (key)
	{
		case TK_ENTER:
			try_use_item();
			break;

		case TK_DELETE:
			try_discard_item();
			break;

		default:
			MenuList::handle_input(key);
			break;
	}
}

void MenuInventory::refresh()
{
	clear_list();
	set_title("Inventory:");

	reserve(Inventory::read().num_items());

	for (int slot = 0; slot < Inventory::read().num_items(); ++slot)
	{
		Item::Handle const item = Inventory::read().peek_item(slot);

		std::string label = (item.stack_height() > 1) ?
			std::format("{} ({})", item.name(), item.stack_height()) :
			item.name();

		add_option(label, slot);
	}
}

void MenuInventory::draw_selected_item()
{
	int const slot = get_selected().value;
	Item::Handle const item = Inventory::read().peek_item(slot);

	if (!item.valid())
	{
		return;
	}

	std::string const name = item.name();
	std::string const description = item.description();
	std::string interaction = item.interaction_name();

	std::stringstream ss;
	ss << item.name() << "\n\n";
	if (!description.empty())
	{
		ss << description << "\n\n";
	}
	if (item.can_use())
	{
		ss << "[[Enter]]  " << interaction << "\n";
	}
	if (item.can_discard())
	{
		ss << "[[Delete]] Discard";
	}

	terminal_print_ext(50, 2, 60, 20, 1, ss.str().c_str());
}

void MenuInventory::try_use_item()
{
	int const slot = get_selected().value;

	Item::Handle const item = Inventory::read().peek_item(slot);
	if (item.can_use())
	{
		Menu::close();
		player_use_item(slot);
	}
}

void MenuInventory::try_discard_item()
{
	int const slot = get_selected().value;

	Item::Handle const item = Inventory::read().peek_item(slot);
	if (item.can_discard())
	{
		Inventory::edit().remove_item(slot);

		if (Inventory::read().num_items() == 0)
		{
			Menu::close();
		}
		else
		{
			refresh();
			m_cursor = std::min(m_cursor, Util::LastIndex(m_options));
		}
	}
}

