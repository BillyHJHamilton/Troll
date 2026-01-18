#pragma once

#include "Types.h"

bool player_try_move(Vec2 const& relative_move);
bool player_try_cast_spell(Spell::Index spell);

void try_cast_spell(Spell::Index spell, Creature::Handle caster, Vec2 target_pos);
