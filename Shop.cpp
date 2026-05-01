#include "Shop.h"

#include "Colour.h"
#include "Creature.h"
#include "Draw.h"
#include "Game.h"
#include "Gingerbread.h"
#include "Loot.h"
#include "Menu.h"
#include "Pathfind.h"
#include "Random.h"
#include "Serialize.h"
#include "VectorUtil.h"
#include "World.h"

namespace Shop
{

//-------------------------------------------------------------------------------------------------
// Data

int constexpr c_ShopItems = 7;
int constexpr c_ActiveTurns = 100;
int constexpr c_InactiveTurns = 150;

Inventory s_shop_inventory;

int s_next_trigger_turn = 0;

//-------------------------------------------------------------------------------------------------
// Interface

void clear()
{
	s_shop_inventory = Inventory{};
}

void serialize(ISerializer& s)
{
	s_shop_inventory.serialize_instance(s);
	s.srz_int(s_next_trigger_turn);
}

Inventory const& read_inventory()
{
	return s_shop_inventory;
}

Inventory& edit_inventory()
{
	return s_shop_inventory;
}

void restock(float difficulty)
{
	// First clean up some old inventory.
	for (int i = 0; i < s_shop_inventory.num_slots(); ++i)
	{
		int const n = s_shop_inventory.peek_item(i).stack_height();
		if (n == 1)
		{
			if (Random::coinflip())
			{
				s_shop_inventory.remove_item(i);
				--i;
				continue;
			}
		}
		else
		{
			int const num_to_remove = Random::in_range(1, n);
			s_shop_inventory.remove_items(i, num_to_remove);
		}
	}

	// Now add new items up to the total.
	int const total_left = s_shop_inventory.total_items();
	int const num_to_add = c_ShopItems - total_left;
	for (int i = 0; i < num_to_add; ++i)
	{
		Item::Handle new_item = Loot::make(Loot::Shop, (Creature::Type)c_Invalid, difficulty);
		s_shop_inventory.add_item(new_item);
	}
}

bool is_active()
{
	Creature::Handle const fred = Gingerbread::find_incarnation(Creature::Fred_Shop);
	Creature::Handle const george = Gingerbread::find_incarnation(Creature::George_Shop);
	return fred.valid() || george.valid();
}

bool try_spawn(Vec3 pos)
{
	if (is_active() ||
		Game::get_turn_number() < s_next_trigger_turn)
	{
		return false;
	}

	Vec3TempList pos_list;
	Pathfind::find_open_neighbours(pos, {.allow_stairs = false}, pos_list);
	if (Util::Size(pos_list) < 2)
	{
		return false;
	}

	Random::shuffle_vector(pos_list);
	Creature::Handle f = Creature::spawn_creature(Creature::Fred_Shop, pos_list.at(0));
	Creature::Handle g = Creature::spawn_creature(Creature::George_Shop, pos_list.at(1));

	float difficulty = World::read().find_map_difficulty(pos);
	restock(difficulty);

	s_next_trigger_turn = Game::get_turn_number() + c_ActiveTurns;

	if (f.visible() || g.visible())
	{
		Draw::add_message("Fred and George emerge from a hidden trapdoor.", cstr_Orange);
	}

	return true;
}

void update()
{
	if (is_active())
	{
		Creature::Handle fred = Gingerbread::find_incarnation(Creature::Fred_Shop);
		Creature::Handle george = Gingerbread::find_incarnation(Creature::George_Shop);

		// Consider if we should despawn.
		if (Game::get_turn_number() >= s_next_trigger_turn &&
			(!fred.valid() || !fred.visible()) &&
			(!george.valid() || !george.visible()))
		{
			fred.destroy();
			george.destroy();
			s_next_trigger_turn = Game::get_turn_number() + c_InactiveTurns;
		}
	}
}

void interact(Creature::Type shop_creature)
{
	if (shop_creature == Creature::Fred_Shop)
	{
		if (s_shop_inventory.read().has_item())
		{
			Menu::show_shop_buy();
		}
		else
		{
			Draw::add_message("Fred says, \"I'm afraid we're out of stock!\"",
				Gingerbread::read(shop_creature).colour);
		}
	}
	else
	{
		// TODO: George has the "Sell" menu
		Draw::add_message("George says, \"Talk to Fred if you'd like to buy.\"",
			Gingerbread::read(shop_creature).colour);
	}
}

}
