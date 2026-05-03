#include "Shop.h"

#include "Colour.h"
#include "Creature.h"
#include "Draw.h"
#include "Game.h"
#include "Geometry.h"
#include "Gingerbread.h"
#include "Loot.h"
#include "Menu.h"
#include "Pathfind.h"
#include "Player.h"
#include "Random.h"
#include "Serialize.h"
#include "Taunt.h"
#include "VectorUtil.h"
#include "World.h"

namespace Shop
{

//-------------------------------------------------------------------------------------------------
// Data

int constexpr c_ShopItems = 9;
int constexpr c_ActiveTurns = 100;
int constexpr c_InactiveTurns = 150;
Interval constexpr c_TalkDelayLong = {10, 20};
Interval constexpr c_TalkDelayShort = {2, 5};
Interval constexpr c_MaxTalk = {2,4};

Inventory s_shop_inventory;

// Normally we close all menus when reloading the game, but closing
// the shop counts as an action, so we need to reopen it on load.
enum class MenuOpen : byte
{
	None,
	Buy,
	Sell
};

// Tracks whether player has bought/sold something.  Affects dialogue.
enum class DealState : byte
{
	NoDeal,
	Deal,
	DealEarlier,
};

struct Data
{
	int next_trigger = 0;
	int next_talk = 0;
	int num_talk = 0;
	Creature::Type last_speaker = Creature::None;
	MenuOpen menu_open = MenuOpen::None;
	DealState deal_state = DealState::NoDeal;
};
Data s_data;

//-------------------------------------------------------------------------------------------------
// Helper function declarations

void do_taunt(Creature::Type type, Taunt::Condition condition);

//-------------------------------------------------------------------------------------------------
// Interface

void clear()
{
	s_shop_inventory = Inventory{};
	s_data = Data{};
}

void serialize(ISerializer& s)
{
	s_shop_inventory.serialize_instance(s);
	s.srz_value(s_data);
}

void post_load()
{
	if (s_data.menu_open == MenuOpen::Buy)
	{
		Menu::show_shop_buy();
	}
	else if (s_data.menu_open == MenuOpen::Sell)
	{
		Menu::show_shop_sell();
	}
};

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
		Game::get_turn_number() < s_data.next_trigger)
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

	s_data.next_trigger = Game::get_turn_number() + c_ActiveTurns;
	s_data.next_talk = Game::get_turn_number() + Random::in_range(c_TalkDelayShort);
	s_data.num_talk = Random::in_range(c_MaxTalk);
	s_data.deal_state = DealState::NoDeal;

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
		if (Game::get_turn_number() >= s_data.next_trigger &&
			(!fred.valid() || !fred.visible()) &&
			(!george.valid() || !george.visible()))
		{
			fred.destroy();
			george.destroy();
			s_data.next_trigger = Game::get_turn_number() + c_InactiveTurns;
		}
	}
}

void interact(Creature::Type shop_creature)
{
	if (shop_creature == Creature::Fred_Shop)
	{
		if (s_shop_inventory.read().has_item())
		{
			s_data.menu_open = MenuOpen::Buy;
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
		if (Inventory::read().has_item_to_sell())
		{
			s_data.menu_open = MenuOpen::Sell;
			Menu::show_shop_sell();
		}
		else
		{
			Draw::add_message("George says, \"Come back if you have something to sell!\"",
				Gingerbread::read(shop_creature).colour);
		}
	}
}

void notify_deal()
{
	s_data.deal_state = DealState::Deal;
}

void notify_menu_close()
{
	Creature::Type const creature_type = (s_data.menu_open == MenuOpen::Buy) ?
		Creature::Fred_Shop :
		Creature::George_Shop;

	if (s_data.deal_state == DealState::Deal)
	{
		do_taunt(creature_type, Taunt::Condition::ShopDeal);
		s_data.deal_state = DealState::DealEarlier;
		s_data.next_talk = std::min(s_data.next_talk,
			Game::get_turn_number() + Random::in_range(c_TalkDelayShort));
	}
	else
	{
		do_taunt(creature_type, Taunt::Condition::ShopNoDeal);
	}

	s_data.menu_open = MenuOpen::None;

	// Pass turn upon leaving the menu.
	Player::set_acted(true);
}

bool has_made_deal()
{
	return s_data.deal_state != DealState::NoDeal;
}

// We don't want Fred and George to exhaust all their dialogue right away,
// so we restrict their talking with several variables.
bool should_talk(Creature::Type speaker)
{
	return s_data.num_talk > 0 &&
		s_data.last_speaker != speaker &&
		Game::get_turn_number() >= s_data.next_talk;
}

void notify_talk(Creature::Type speaker)
{
	s_data.last_speaker = speaker;
	s_data.next_talk = Game::get_turn_number() + Random::in_range(c_TalkDelayLong);
	--s_data.num_talk;
}

//-------------------------------------------------------------------------------------------------
// Helper function implementations

void do_taunt(Creature::Type type, Taunt::Condition condition)
{
	Creature::Handle creature = Gingerbread::find_incarnation(type);
	if (creature.valid())
	{
		IntTempList taunts;
		Taunt::find_taunts(creature, condition, c_Invalid, taunts);
		if (!taunts.empty())
		{
			Taunt::say_taunt(creature, Random::from_vector(taunts));
		}
	}
}

} // namespace Shop
