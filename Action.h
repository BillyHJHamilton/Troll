#pragma once

#include "Types.h"

void player_rest_step();
bool player_try_move(Vec2 relative_move);
bool player_try_cast_spell(Spell::Index spell);

bool try_move (Creature::Handle creature, Vec2 relative_move, MoveMode move_mode);

// Use line_id = c_invalid for self casting.
void try_cast_spell(Spell::Index spell, Creature::Handle caster, Vec3 target_pos, int line_id);
