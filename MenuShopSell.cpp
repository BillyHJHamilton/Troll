#include "MenuShopSell.h"

#include "Colour.h"
#include "Debug.h"
#include "Inventory.h"
#include "Shop.h"
#include "VectorUtil.h"

#include "BearLibTerminal.h"
#include <format>

void MenuShopSell::draw_screen()
{
	MenuList::draw_screen();

	draw_selected_item();
}

Input::Result MenuShopSell::handle_input (int key)
{
	switch (key)
	{
		case TK_ENTER:
			sell_item();
			return Input::Result::Handled;

		default:
			return MenuList::handle_input(key);
	}
}

void MenuShopSell::on_close()
{
	Shop::notify_menu_close();
}

void MenuShopSell::refresh()
{
	clear_list();
	
	Inventory const& player_inventory = Inventory::read();
	int const player_beans = player_inventory.num_beans();
	set_title(std::format("What would you like to sell?\n"
		"You have {} beans.",
		player_beans));
		
	reserve(player_inventory.num_slots());

	for (int slot = 0; slot < player_inventory.num_slots(); ++slot)
	{
		Item::Handle const item = player_inventory.peek_item(slot);

		if (!item.can_sell())
		{
			continue;
		}

		int const price = item.sell_price();

		std::string label = std::format("{} beans - {}", price, item.name());
		if (item.stack_height() > 1)
		{
			label += std::format(" ({})", item.stack_height());
		}

		add_option(label, slot);
	}
}

void MenuShopSell::draw_selected_item()
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
	ss << "[[Enter]]  Sell\n";
	ss << "[[Esc]]    Cancel\n";

	terminal_print_ext(50, 2, 60, 20, 1, ss.str().c_str());
}

void MenuShopSell::sell_item()
{
	Inventory& shop_inventory = Shop::edit_inventory();
	Inventory& player_inventory = Inventory::edit();

	int const slot = get_selected().value;
	Item::Handle const item = player_inventory.peek_item(slot);
	int const price = item.sell_price();

	for (int i = 0; i < price; ++i)
	{
		player_inventory.add_item(Item::make_bbb());
	}

	Item::Handle sold_item = player_inventory.pop_item(slot);
	shop_inventory.add_item(sold_item);

	Shop::notify_deal();

	if (!player_inventory.has_item_to_sell())
	{
		Menu::close();
	}
	else
	{
		int const previous_cursor = m_cursor;
		refresh();
		m_cursor = std::min(Util::LastIndex(m_options), previous_cursor);
	}
}
