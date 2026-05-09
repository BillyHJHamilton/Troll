#include "MenuShopBuy.h"

#include "Colour.h"
#include "Debug.h"
#include "Inventory.h"
#include "Shop.h"
#include "VectorUtil.h"

#include "BearLibTerminal.h"
#include <format>

void MenuShopBuy::draw_screen()
{
	MenuList::draw_screen();

	draw_selected_item();
}

Input::Result MenuShopBuy::handle_input (int key)
{
	switch (key)
	{
		case TK_ENTER:
			try_buy_item();
			return Input::Result::Handled;

		default:
			return MenuList::handle_input(key);
	}
}

void MenuShopBuy::on_close()
{
	Shop::notify_menu_close();
}

void MenuShopBuy::refresh()
{
	clear_list();
	
	int const player_beans = Inventory::read().num_beans();
	set_title(std::format("What would you like to buy?\n"
		"You have {} beans.",
		player_beans));
		
	Inventory const& shop_inventory = Shop::read_inventory();
	reserve(shop_inventory.num_slots());

	for (int slot = 0; slot < shop_inventory.num_slots(); ++slot)
	{
		Item::Handle const item = shop_inventory.peek_item(slot);
		int const price = item.buy_price();

		std::string label = std::format("{} beans - {}", price, item.name());
		if (item.stack_height() > 1)
		{
			label += std::format(" ({})", item.stack_height());
		}

		char const* colour = (player_beans >= price) ?
			cstr_White :
			cstr_DarkGrey;

		add_option(label, slot, colour);
	}
}

void MenuShopBuy::draw_selected_item()
{
	int const slot = get_selected().value;
	Item::Handle const item = Shop::read_inventory().peek_item(slot);

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
	ss << "[[Enter]]  Buy\n";
	ss << "[[Esc]]    Cancel\n";

	terminal_print_ext(50, 2, 60, 20, 1, ss.str().c_str());
}

void MenuShopBuy::try_buy_item()
{
	Inventory& shop_inventory = Shop::edit_inventory();
	Inventory& player_inventory = Inventory::edit();

	int const slot = get_selected().value;
	Item::Handle const item = shop_inventory.peek_item(slot);
	int const price = item.buy_price();
	int const player_beans = player_inventory.num_beans();

	if (player_beans >= price)
	{
		int const bean_slot = player_inventory.find_first_item(Item::BBBean);
		player_inventory.remove_items(bean_slot, price);

		Item::Handle bought_item = shop_inventory.pop_item(slot);
		player_inventory.add_item(bought_item);

		Shop::notify_deal();

		if (!shop_inventory.has_item())
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
}
