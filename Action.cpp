#include "Action.h"

#include "Beam.h"
#include "Creature.h"
#include "Draw.h"
#include "Grammar.h"
#include "Map.h"
#include "Player.h"
#include "Random.h"
#include "Spell.h"
#include "Target.h"

#include <cassert>

//-------------------------------------------------------------------------------------------------
// Helper function declarations

bool check_distraction (int caster);
bool check_miscast (int caster, Spell::Index spell);
void do_miscast (int caster, Spell::Index spell, Vec2 target_pos);
void do_successful_cast (int caster, Spell::Index spell, Vec2 target_pos);

//-------------------------------------------------------------------------------------------------
// Interface functions

bool player_try_cast_spell (Spell::Index spell)
{
	// check if the player knows the spell
	if (!creature_knows_spell(Creature::Player, spell))
	{
		add_game_message("You don't know that spell.");
		return false;
	}

	// targeting
	std::optional<Vec2> target_pos = get_target_pos();

	if (!target_pos.has_value())
	{
		add_game_message("No target.");
		return false;
	}

	// check for self-targeting
	if (target_pos == Player::pos() && Spell::get_accuracy(spell) != -1)
	{
		add_game_message("Don't shoot that spell at yourself.");
		return false;
	}

	// Having confirmed it is plausible for the player to try to cast the spell,
	// we now continue to the generic spell-casting function
	try_cast_spell(spell, Creature::Player, *target_pos);

	return true;
}

void try_cast_spell (Spell::Index spell, int caster, Vec2 target_pos)
{
	// Update the screen because we'll do some animation for the spell
	draw_screen();

	// 1. Distractedness
	bool is_distracted = check_distraction(caster);
	if (is_distracted)
	{
		add_game_message(Grammar::You_are(caster) + " too distracted to cast a spell!");
		return;
	}

	// 2. Deduct hatred for dark spells (even on miscast)
	// Todo

	// 3. Miscastiness
	bool is_miscast = check_miscast(caster, spell);
	if ( is_miscast )
	{
		return;
	}

	// 4. Success!
	do_successful_cast(caster, spell, target_pos);
}

//-------------------------------------------------------------------------------------------------
// Helper function implementations

bool check_distraction (int caster)
{
	// Simple percentage chance you won't get to cast at all
	int distractedness = creature_distractedness(caster); 
	int distractedness_roll = random(0,99);
	if (SHOW_SPELL_DEBUG)
	{
		std::cout << "Distraction Rate: " << distractedness
			 << "%    Roll: " << distractedness_roll << std::endl << " ";
	}

	return (distractedness_roll < distractedness);
}

bool check_miscast (int caster, Spell::Index spell)
{
	// Chance you mess up the spell, based on its difficulty and your skill.
	// This is done with float math since there's an exponent in the formula.
	float miscast_rate = creature_miscast_rate_for_spell(caster, spell);
	float miscast_roll = random(0.0f, 100.0f);

	if (SHOW_SPELL_DEBUG)
	{
		std::cout << "Miscast Rate: " << miscast_rate
			 << "%    Roll: " << miscast_roll << std::endl << " ";
	}

	if (miscast_roll < miscast_rate)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void do_miscast (int caster, Spell::Index spell, Vec2 target_pos)
{
	if (creature_visible(caster))
	{
		std::string message = Grammar::You(caster) + " ";
		message += Grammar::verbs("miscast", caster) + " ";
		message += Spell::get_name(spell) + "!";
		add_game_message(std::move(message));

		DrawView view = get_draw_view();
		draw_tile_temp('X', creature_pos(caster), view, "yellow");
		draw_tile_temp('X', creature_pos(caster), view, "black");
	}

	// todo - proper miscasts
	//Miscast::perform(caster, target, spell_used);
}

void do_successful_cast (int caster, Spell::Index spell, Vec2 target_pos)
{
	std::string message;
	if (creature_is_player(caster))
	{
		message = "You cast " + Spell::get_name(spell) + "!";
	}
	else
	{
		message = creature_name(caster) + " casts " + Spell::get_name(spell) + ".";
	}
	add_game_message(std::move(message));

	if (Spell::get_accuracy(spell) == -1) // self-affecting spell
	{
		Spell::execute_effect(spell, caster, Creature::None);
	}
	else
	{
		bool constexpr caster_aimed = true;
		Beam::shoot_spell (spell, caster, target_pos, caster_aimed);
	}
}
