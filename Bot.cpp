#include "Bot.h"

#include "Action.h"
#include "Beam.h"
#include "Creature.h"
#include "Debug.h"
#include "Draw.h"
#include "Geometry.h"
#include "Grammar.h"
#include "Math.h"
#include "Pathfind.h"
#include "Player.h"
#include "Random.h"
#include "Serialize.h"
#include "Spell.h"
#include "Stairs.h"
#include "VectorUtil.h"
#include "World.h"

#include <format>

namespace Bot
{

static bool constexpr TERMINATOR_MODE = false;

// Number of turns a bot will remain "aware" after losing sight of player.
static int constexpr c_MaxAwareness = 10;

// Maximum path length to consider when using a-star pathfinding.
static int constexpr c_MaxPathCost = 25;

static std::vector<Brain> s_brains;

// ------------------------------------------------------------------------------------------------
// Helper function declarations

bool is_aware(Creature::Handle const creature);
void go_to_last_seen(Creature::Handle creature);
void move_towards(Creature::Handle creature, Vec3 dest);
bool try_follow_path(Creature::Handle creature, std::vector<Vec3>& move_stack);
Spell::Index choose_spell (Creature::Handle caster, Creature::Handle target);
Spell::Index highest_predicted_damage_spell (Creature::Handle caster, Creature::Handle target,
	std::vector<Spell::Index> const & spell_list);
float estimated_damage_output (Spell::Index spell, Creature::Handle caster, Creature::Handle target);
bool spell_is_useless (Spell::Index spell, Creature::Handle caster, Creature::Handle target);

// ------------------------------------------------------------------------------------------------
// Interface functions

void clear()
{
	s_brains.clear();
}

void Brain::serialize(ISerializer& s)
{
	s.srz_int(awareness);
	s.srz_vec3(last_seen);

	srz_vector(s, move_stack, "brain.move_stack");
}

void serialize(ISerializer& s)
{
	srz_vector_size(s, s_brains, "brains");
	for (Brain& b : s_brains)
	{
		b.serialize(s);
	}
}

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
	Vec3 const pos = creature.pos();

	World const& world = World::read();
	int line_id = world.get_los(pos, Player::pos(), creature_vision);
	bool const player_is_visible = (line_id != c_Invalid);

	if (player_is_visible)
	{
		if (!is_aware(creature))
		{
			// Spend a turn noticing the player.
			Draw::creature_message(creature, Grammar::You(creature) + " sees you!");

			// And break automove since player probably wants to respond to this.
			Player::stop_automove();
		}
		else
		{
			std::vector<Spell::Index> spell_list = creature.spells_known();
			if (spell_list.size() > 0)
			{
				Spell::Index spell = choose_spell(creature, Player::handle());
				//Spell::Index spell = Random::from_vector(spell_list);
				if (within_range(creature.pos(),
					Player::pos(), Spell::get_range(spell)))
				{
					try_cast_spell(spell, creature, Player::pos(), line_id);
				}
				else
				{
					move_towards(creature, Player::pos());
				}
			}
		}

		brain.awareness = c_MaxAwareness;
		brain.last_seen = Player::pos();
	}
	else
	{
		if (TERMINATOR_MODE) // hunt down player (for testing)
		{
			brain.awareness = 10;
			brain.last_seen = Player::pos();
		}

		if (is_aware(creature))
		{
			--brain.awareness;
			go_to_last_seen(creature);
		}
		else
		{
			creature.rest_step();
		}
	}
}

// ------------------------------------------------------------------------------------------------
// Helper function implementations

bool is_aware(Creature::Handle const creature)
{
	return s_brains[creature].awareness > 0;
}

// Tries to use pathfinding, or falls back to the basic move towards.
void go_to_last_seen(Creature::Handle creature)
{
	Brain& brain = s_brains[creature];
	Vec3 const pos = creature.pos();

	if (pos == brain.last_seen)
	{
		Stairs::Direction dir = World::read().get_stairs(creature.pos());
		if (dir != Stairs::None)
		{
			// Hm, where could she possibly have gone?
			try_move(creature, Stairs::relative_move(dir).xy(), MoveMode::Walk);
		}
		else
		{
			// TODO: Explore a little.  Try to find the player.
			creature.rest_step();
		}
	}
	else if (pos.z == brain.last_seen.z
		&& chessboard_distance(pos.xy(), brain.last_seen.xy()) == 1)
	{
		// Only one square away, you can do it!
		move_towards(creature, brain.last_seen);
	}
	else
	{
		bool moved = false;

		if (!brain.move_stack.empty())
		{
			// We have a plan.  See if we can still follow it.
			moved = try_follow_path(creature, brain.move_stack);
		}

		if (!moved)
		{
			// Try to formulate a new plan.
			brain.move_stack = Pathfind::astar(pos, brain.last_seen, c_MaxPathCost);

			if (!brain.move_stack.empty())
			{
				moved = try_follow_path(creature, brain.move_stack);
			}
		}
				
		if (!moved)
		{
			move_towards(creature, brain.last_seen);
		}
	}
}

// A naïve move straight towards the destination.
void move_towards(Creature::Handle creature, Vec3 dest)
{
	Vec3 const to_dest = dest - creature.pos();
	Vec2 const move_dir = {
		Math::Sign(to_dest.x),
		Math::Sign(to_dest.y)
	};

	bool moved = try_move(creature, move_dir, MoveMode::Walk);

	if (!moved)
	{
		moved = try_move(creature, { move_dir.x, 0 }, MoveMode::Walk);
	}

	if (!moved)
	{
		moved = try_move(creature, { 0, move_dir.y }, MoveMode::Walk);
	}
}

bool try_follow_path(Creature::Handle creature, std::vector<Vec3>& move_stack)
{
	// Try to follow our plan
	Vec3 const next_pos = Util::PopBack(move_stack);
	Vec2 const next_move = next_pos.xy() - creature.pos().xy();
	if (next_move.x >= -1 && next_move.x <= 1 && next_move.y >= -1 && next_move.y <= 1)
	{
		return try_move(creature, next_move, MoveMode::Walk);
	}

	return false;
}

Spell::Index choose_spell (Creature::Handle caster, Creature::Handle target)
{
	Spell::Index spell_chosen = Spell::None;
	std::vector<Spell::Index> spell_list = caster.spells_known();

	// 50% chance of doing attack with best predicted damage
	if (Random::coinflip())
	{
		spell_chosen = highest_predicted_damage_spell(caster, target, spell_list);
		if (spell_chosen != Spell::None)
		{
			if (c_ShowBotDebug)
			{
				std::cout << std::format("{} using highest output spell = {}\n",
					caster.short_name(), Spell::get_name(spell_chosen));
			}

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

	Random::shuffle_vector(spell_list);

	for (int i = 0; i < spell_list.size(); ++i)
	{
		spell_chosen = spell_list[i];
		if ( !spell_is_useless(spell_chosen, caster, target)
			 && Random::in_range(0.0f, 100.0f) > caster.miscast_rate_for_spell(spell_chosen) )
		{
			if (c_ShowBotDebug)
			{
				std::cout << std::format("{} using random reasonable spell = {}\n",
					caster.short_name(), Spell::get_name(spell_chosen));
			}

			return spell_chosen;
		}
	}

	if (c_ShowBotDebug)
	{
		std::cout << std::format("{} couldn't find good spell.  Using random = {}\n",
			caster.short_name(), Spell::get_name(spell_chosen));
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

	// Non-combat spells
	if (spell == Spell::Alohomora)
	{
		return true;
	}
	
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
