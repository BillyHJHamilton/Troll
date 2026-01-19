#include "Bot.h"

#include "Action.h"
#include "Beam.h"
#include "Creature.h"
#include "Draw.h"
#include "Global.h"
#include "Grammar.h"
#include "Map.h"
#include "Math.h"
#include "Player.h"
#include "Random.h"
#include "Spell.h"
#include "VectorUtil.h"

namespace Bot
{

static bool constexpr SHOW_BOT_DEBUG = true;

// Number of turns a bot will remain "aware" after losing sight of player.
static int constexpr c_max_awareness = 10;

static std::vector<Brain> s_brains;

// ------------------------------------------------------------------------------------------------
// Helper function declarations

bool is_aware(Creature::Handle const creature);
void move_towards(Creature::Handle creature, Vec2 dest);
Spell::Index choose_spell (Creature::Handle caster);
Spell::Index highest_predicted_damage_spell (Creature::Handle caster, Creature::Handle target,
	std::vector<Spell::Index> const & spell_list);
float estimated_damage_output (Spell::Index spell, Creature::Handle caster, Creature::Handle target);
bool spell_is_useless (Spell::Index spell, Creature::Handle caster, Creature::Handle target);

// ------------------------------------------------------------------------------------------------
// Interface functions

void init_brain(Creature::Handle handle)
{
	// Technically we leave an empty brain for the player.
	// It's not worth the confusingness of offsetting the indices.

	if (Util::IsValidIndex(s_brains, handle))
	{
		s_brains[handle] = Brain{};
	}
	else
	{
		s_brains.reserve(handle + 1);
		while (s_brains.size() < handle + 1)
		{
			s_brains.emplace_back();
		}
	}
}

void do_turn (Creature::Handle creature)
{
	constexpr int creature_vision = 8; // Add variable/function later if desired.
	Brain& brain = s_brains[creature];

	bool player_is_visible = check_los(g_map(), creature.pos(), Player::pos())
		&& check_within_range(creature.pos(), Player::pos(), creature_vision);

	if (player_is_visible)
	{
		if (!is_aware(creature))
		{
			// Spend a turn noticing the player.
			add_game_message(Grammar::You(creature) + " sees you!");
		}
		else
		{
			std::vector<Spell::Index> spell_list = creature.spells_known();
			if (spell_list.size() > 0)
			{
				Spell::Index spell = random_from_vector(spell_list);
				if (check_within_range(creature.pos(),
					Player::pos(), Spell::get_range(spell)))
				{
					try_cast_spell(spell, creature, Player::pos());
				}
				else
				{
					move_towards(creature, Player::pos());
				}
			}
		}

		brain.awareness = c_max_awareness;
		brain.last_seen = Player::pos();
	}
	else
	{
		if (is_aware(creature))
		{
			--brain.awareness;
			move_towards(creature, brain.last_seen);
		}
	}
}

// ------------------------------------------------------------------------------------------------
// Helper function implementations

bool is_aware(Creature::Handle const creature)
{
	return s_brains[creature].awareness > 0;
}

void move_towards(Creature::Handle creature, Vec2 dest)
{
	// TODO proper pathfinding

	Vec2 const to_dest = dest - creature.pos();
	Vec2 const move_dir = {
		Math::Sign(to_dest.x),
		Math::Sign(to_dest.y)
	};

	bool moved = creature.try_move(move_dir, MoveMode::Walk);

	if (!moved)
	{
		moved = creature.try_move({ move_dir.x, 0 }, MoveMode::Walk);
	}

	if (!moved)
	{
		moved = creature.try_move({ 0, move_dir.y }, MoveMode::Walk);
	}
}

Spell::Index choose_spell (Creature::Handle caster)
{
	Creature::Handle target = Creature::Player;
	Spell::Index spell_chosen = Spell::None;
	std::vector<Spell::Index> spell_list = caster.spells_known();

	// 50% chance of doing attack with best predicted damage
	if (coinflip())
	{
		if (SHOW_BOT_DEBUG)
		{
			std::cout << "Doing highest output spell." << std::endl;
		}
		spell_chosen = highest_predicted_damage_spell(caster, target, spell_list);
		if (spell_chosen != Spell::None)
		{
			return spell_chosen;
		}
	}

	// if not, and shield available, good chance of doing that
	//if (random (2.0) < 1.0
	//	&& caster.spell_castable(Spell::Protego)
	//    && caster.get_miscast_rate(Spell::Protego) < 40)
	//{
	//	//cout << "Doing shield." << endl;
	//	return Spell::Protego;
	//}

	// otherwise do something random with reasonable chance of success
	if (spell_list.size() == 0)
	{
		return Spell::None;
	}

	if (SHOW_BOT_DEBUG)
	{
		std::cout << "Doing random spell." << std::endl;
	}

	int i = 0;
	int num_tries = static_cast<int>(5*spell_list.size());
	while (i < num_tries)
	{
		spell_chosen = random_from_vector(spell_list);
		if ( !spell_is_useless(spell_chosen, caster, target)
			 && random(0.0f, 100.0f) > caster.miscast_rate_for_spell(spell_chosen) )
		{
			return spell_chosen;
		}
		else
		{
			i++;
		}
	}

	if (SHOW_BOT_DEBUG)
	{
		std::cout << "Couldn't find any reasonable spell after " << num_tries
			<< "tries." << std::endl;
	}

	// if can't find anything that might succeed, give up and just cast whatever
	return spell_chosen;
}

Spell::Index highest_predicted_damage_spell (Creature::Handle caster, Creature::Handle target,
	std::vector<Spell::Index> const & spell_list)
{
	Spell::Index best_spell = Spell::None;
	float best_estimate = 0.0f;
	for (Spell::Index spell : spell_list)
	{
		float estimate = estimated_damage_output(spell, caster, target);
		if (estimate > best_estimate)
		{
			best_estimate = estimate;
			best_spell = spell;
		}
	}

	return best_spell;
}

float estimated_damage_output (Spell::Index spell, Creature::Handle caster, Creature::Handle target)
{
	float estimate;

	// Special cases go here.
	// We could also allow spells to define a damage estimator function...

	// Normal spell
	estimate = static_cast<float>(Spell::get_damage(spell, caster));

	// Factor in accuracy
	int const accuracy = Beam::accuracy_at_range(Spell::get_accuracy(spell),
		caster.pos(), target.pos());
	estimate = estimate * (accuracy / 100.0f);

	// Factor in miscast rate
	float good_cast_rate = 100.0f - caster.miscast_rate_for_spell(spell);
	estimate = estimate * (good_cast_rate / 100.0f);

	// We don't factor in distraction, because it is the same for all spells

	if (estimate < 0)
	{
		estimate = 0;
	}

	return estimate;
}

bool spell_is_useless (Spell::Index spell, Creature::Handle caster, Creature::Handle target)
{
	// if we have virtually no chance of casting it, it's useless
	float miscast_rate = caster.miscast_rate_for_spell(spell);
	if (miscast_rate >= 99.0)
		return true;
	
	// finite inc. is useless if there's no enchantment to break
	//if (spell_index == Spell::FINITE_INC)
	//{
	//	if ( !caster.has_status(Status::BATTY)
	//		&& !caster.has_status(Status::CONFUSED)
	//		&& !caster.has_status(Status::DANCING)
	//		&& !caster.has_status(Status::IMPEDED)
	//		&& !caster.has_status(Status::LEG_LOCKED)
	//		&& !caster.has_status(Status::TICKLED)
	//		&& !caster.has_status(Status::TONGUE_TIED) )
	//	{
	//		return true;
	//	}
	//}

	//// oppugno ohne birds is useless
	//if (spell_index == Spell::OPPUGNO
	//	&& !caster.has_status(Status::BIRDS) )
	//{
	//	return true;
	//}

	//// avis at max birds is useless
	//if (spell_index == Spell::AVIS
	//	&& caster.status_severity(Status::BIRDS) >= 10 )
	//{
	//	return true;
	//}

	// otherwise the spell could conceivably work in some circumstance.
	return false;
}

} // namespace Bot
