#pragma once

#include "Types.h"

namespace Action
{
	void player_look_at();
	
	void player_rest_step();
	bool player_try_move(Vec2 relative_move);
	bool player_try_cast_spell(Spell::Index spell);
	void player_use_item(int inventory_slot);

	bool is_move_hazardous (Creature::Handle creature, Vec2 relative_move);
	bool try_move (Creature::Handle creature, Vec2 relative_move, MoveMode move_mode);

	// Use line_id = c_Invalid if spell is self-targeted or sight targeted.
	void try_cast_spell(Spell::Index spell, Creature::Handle caster, Vec3 target_pos, int line_id);
	void try_use_ability(Ability::Index ability, Creature::Handle user, Vec3 target_pos, int line_id);
}