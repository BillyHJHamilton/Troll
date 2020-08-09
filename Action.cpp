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

bool player_try_cast_spell(Spell::Index spell)
{
	// check if the player knows the spell
	// todo

	// targeting
	Vec2 target_pos = {0,0};
	if (g_target_mode == TargetMode::Automatic)
	{
		if (creature_valid(g_target_index))
		{
			target_pos = creature_pos(g_target_index);
		}
		else
		{
			add_game_message("No target.");
			return false;
		}
	}
	else if (g_target_mode == TargetMode::Manual)
	{
		target_pos = g_target_pos;
	}
	else
	{
		assert(false); // unhandled case
	}

	// check if targeting is valid...
	if (target_pos == Player::pos() && Spell::get_accuracy(spell) != -1)
	{
		add_game_message("Don't shoot that spell at yourself.");
		return false;
	}

	// Having confirmed it is plausible for the player to try to cast the spell,
	// we now continue to the generic spell-casting function
	try_cast_spell(spell, Creature::Player, target_pos);

	return true;
}

void try_cast_spell(Spell::Index spell, int caster, Vec2 target_pos)
{
	// Update the screen because we'll do some animation for the spell
	draw_screen();

	// 1. Distractedness
	// Chance you won't get to cast at all

	int distractedness = creature_distractedness(caster); 
	int distractedness_roll = random(0,99);
	if (SHOW_SPELL_DEBUG)
	{
		std::cout << "Distraction Rate: " << distractedness
			 << "%    Roll: " << distractedness_roll << std::endl << " ";
	}

	if (distractedness_roll < distractedness )
	{
		add_game_message(Grammar::Name_is(caster) + " too distracted to cast a spell!");
		return;
	}

	// 2. Deduct hatred for dark spells (even on miscast)
	// Todo

	// 3. Miscastiness
	// Chance you mess up the spell, based on its difficulty and your skill.
	// This is done with float math since there's an exponent in the formula.

	// Miscastiness effectively applies a penalty to your magic skill.
	int miscastiness = creature_miscastiness(caster);
	int skill_magic = creature_skill_magic(caster);
	int effective_skill_magic = skill_magic - miscastiness;

	float miscast_rate = Spell::get_miscast_rate(spell, effective_skill_magic);
	float miscast_roll = random(0.0f, 100.0f);
	if (SHOW_SPELL_DEBUG)
	{
		std::cout << "Miscast Rate: " << miscast_rate
			 << "%    Roll: " << miscast_roll << std::endl << " ";
	}

	bool is_miscast;
	if (miscast_roll < miscast_rate)
		is_miscast = true;
	// exception: underwater, every cast is a miscast!
	//else if ( caster.has_status(Status::WATER) )
	//	is_miscast = true;
	else
		is_miscast = false;

	if ( is_miscast )
	{
		if (creature_visible(caster))
		{
			std::string message = Grammar::Name(caster) + " ";
			message += Grammar::verbs("miscast", caster) + " ";
			message += Spell::get_name(spell) + "!";
			add_game_message(std::move(message));

			DrawView view = get_draw_view();
			draw_tile_temp('X', creature_pos(caster), view, "yellow");
			draw_tile_temp('X', creature_pos(caster), view, "black");
		}

		// todo - proper miscasts
		//Miscast::perform(caster, target, spell_used);
		return;
	}

	// Display message on successful cast.

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
