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

bool check_distraction (Creature::Handle caster);
bool check_miscast (Creature::Handle caster, Spell::Index spell);
void do_miscast (Creature::Handle caster, Spell::Index spell, Vec2 target_pos);
void do_successful_cast (Creature::Handle caster, Spell::Index spell, Vec2 target_pos);

//-------------------------------------------------------------------------------------------------
// Interface functions

bool player_try_move(Vec2 const& relative_move)
{
	bool const moved = g_player().handle().try_move(relative_move, MoveMode::Walk);
	if (moved)
	{
		g_player().acted = true;
	}
	return moved;
}

bool player_try_cast_spell (Spell::Index spell)
{
	// check if the player knows the spell
	if (!Player::handle().knows_spell(spell))
	{
		Draw::add_message("You don't know that spell.");
		return false;
	}

	// targeting
	std::optional<Vec2> target_pos = Target::get_pos();

	if (!target_pos.has_value())
	{
		Draw::add_message("No target.");
		return false;
	}

	// check for self-targeting
	if (target_pos == Player::pos() && Spell::get_accuracy(spell) != -1)
	{
		Draw::add_message("Don't shoot that spell at yourself.");
		return false;
	}

	// check for out of range
	if (!check_within_range(Player::pos(), target_pos.value(), Spell::get_range(spell)))
	{
		Draw::add_message("The target is out of range.");
		return false;
	}

	// Having confirmed it is plausible for the player to try to cast the spell,
	// we now continue to the generic spell-casting function
	try_cast_spell(spell, Creature::Player, *target_pos);

	g_player().acted = true;
	return true;
}

void try_cast_spell (Spell::Index spell, Creature::Handle caster, Vec2 target_pos)
{
	// Update the screen because we'll do some animation for the spell
	Draw::draw_screen();

	// 1. Distractedness
	bool is_distracted = check_distraction(caster);
	if (is_distracted)
	{
		Draw::add_message(Grammar::You_are(caster) + " too distracted to cast a spell!");
		return;
	}

	// 2. Deduct hatred for dark spells (even on miscast)
	// Todo

	// 3. Miscastiness
	bool is_miscast = check_miscast(caster, spell);
	if ( is_miscast )
	{
		do_miscast(caster, spell, target_pos);
		return;
	}

	// 4. Success!
	do_successful_cast(caster, spell, target_pos);
}

//-------------------------------------------------------------------------------------------------
// Helper function implementations

bool check_distraction (Creature::Handle caster)
{
	// Simple percentage chance you won't get to cast at all
	int distraction_rate = caster.distractedness();

	// Always a chance...
	if (distraction_rate > 90)
	{
		distraction_rate = 90;
	}

	int distractedness_roll = Random::in_range(0,99);
	if (SHOW_SPELL_DEBUG)
	{
		std::cout << "Distraction Rate: " << distraction_rate
			 << "%    Roll: " << distractedness_roll << std::endl << " ";
	}

	return (distractedness_roll < distraction_rate);
}

bool check_miscast (Creature::Handle caster, Spell::Index spell)
{
	// Chance you mess up the spell, based on its difficulty and your skill.
	// This is done with float math since there's an exponent in the formula.
	float miscast_rate = caster.miscast_rate_for_spell(spell);
	float miscast_roll = Random::in_range(0.0f, 100.0f);

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

void do_miscast (Creature::Handle caster, Spell::Index spell, Vec2 target_pos)
{
	if (caster.visible())
	{
		std::string message = Grammar::You(caster) + " ";
		message += Grammar::verbs("miscast", caster) + " ";
		message += Spell::get_name(spell) + "!";
		Draw::add_message(std::move(message));

		Draw::View view = Draw::get_view();
		Draw::draw_tile_temp('X', caster.pos(), view, "yellow");
		Draw::draw_tile_temp('X', caster.pos(), view, "black");
	}

	// todo - proper miscasts
	//Miscast::perform(caster, target, spell_used);
}

void do_successful_cast (Creature::Handle caster, Spell::Index spell, Vec2 target_pos)
{
	std::string message = Grammar::You(caster) + " " + Grammar::verbs("cast", caster)
		+ " " + Spell::get_name(spell) + "!";
	Draw::add_message(std::move(message));

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
