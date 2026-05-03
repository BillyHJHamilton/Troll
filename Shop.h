#pragma once

#include "Types.h"
#include "Inventory.h"

namespace Shop
{
	void clear();
	void serialize(ISerializer& s);
	void post_load();

	Inventory const& read_inventory();
	Inventory& edit_inventory();

	void restock(float difficulty);

	bool is_active();
	bool try_spawn(Vec3 at);
	void update();
	void interact(Creature::Type shop_creature);

	void notify_deal();
	void notify_menu_close();

	Vec3 get_tether_pos();
	bool has_made_deal();
	bool should_talk(Creature::Type speaker);
	void notify_talk(Creature::Type speaker);
};
