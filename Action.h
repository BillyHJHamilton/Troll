#pragma once

#include "Types.h"

bool player_try_cast_spell(char const * spell_abbrev);

void try_cast_spell(Spell::Index spell, int caster, Vec2 target_pos);
