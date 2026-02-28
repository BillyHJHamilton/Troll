#include "Bot.h"

#include "Action.h"
#include "Beam.h"
#include "Creature.h"
#include "Debug.h"
#include "Draw.h"
#include "Game.h"
#include "Geometry.h"
#include "Grammar.h"
#include "Math.h"
#include "Pathfind.h"
#include "Player.h"
#include "Random.h"
#include "Serialize.h"
#include "Spell.h"
#include "Stairs.h"
#include "Status.h"
#include "VectorUtil.h"
#include "World.h"

#include <format>

namespace Bot
{

static bool constexpr c_TerminatorMode = false;

// Constant for now.  Can add variable/function later if desired.
constexpr int c_CreatureVision = 8;

// Number of turns a bot will remain "aware" after losing sight of player.
static int constexpr c_MaxAwareness = 15;

// Maximum path length to consider when using a-star pathfinding.
static int constexpr c_MaxPathCost = 25;

// After using the pathfinder, AI isn't allowed to use it again for this many turns.
// For performance reasons, and so they don't change directions wildly.
static int constexpr c_PathfindCooldown = 5;

static std::vector<Brain> s_brains;

// Data used by bot during a single turn.
struct Thoughts
{
	bool target_visible = false;
	int target_line = c_Invalid;
};

// ------------------------------------------------------------------------------------------------
// Helper function declarations

void check_for_target(Creature::Handle creature, Brain& brain, Thoughts& thoughts);
void check_transitions(Creature::Handle creature, Brain& brain, Thoughts& thoughts);

void bot_blunder(Creature::Handle creature, Brain& brain, Thoughts& thoughts);
void bot_chase(Creature::Handle creature, Brain& brain, Thoughts& thoughts);
void bot_fight(Creature::Handle creature, Brain& brain, Thoughts& thoughts);

bool is_aware(Creature::Handle const creature);
bool try_move_towards(Creature::Handle creature, Vec3 dest);
bool try_follow_path(Creature::Handle creature, std::vector<Vec3>& move_stack);
Spell::Index choose_spell (Creature::Handle caster, Creature::Handle target);
Spell::Index highest_predicted_damage_spell (Creature::Handle caster, Creature::Handle target,
	Spell::TempList const & spell_list);
float estimated_damage_output (Spell::Index spell, Creature::Handle caster, Creature::Handle target);
bool spell_is_useless (Spell::Index spell, Creature::Handle caster, Creature::Handle target);

// ------------------------------------------------------------------------------------------------
// Interface functions

void init()
{
	s_brains.reserve(Creature::c_MaxCreatures);
}

void clear()
{
	s_brains.clear();
}

void Brain::serialize(ISerializer& s)
{
	s.srz_vector(move_stack, "brain.move_stack");
	s.srz_vec3(target_pos);
	s.srz_value(target);
	s.srz_int(awareness);
	s.srz_int(patience);
	s.srz_value(state);
}

void serialize(ISerializer& s)
{
	s.srz_vector_advanced(s_brains, "brains");
}

void init_brain(Creature::Handle handle)
{
	// Technically we leave an empty brain for the player.
	// It's not worth the confusingness of offsetting the indices.

	if (Util::IsValidIndex(s_brains, handle))
	{
		s_brains[handle] = Brain{}; // TODO I fear this reallocates the move_stack vector
	}
	else
	{
		while (s_brains.size() < handle + 1)
		{
			s_brains.emplace_back();
		}
	}
}

Brain& get_brain(Creature::Handle handle)
{
	if (!Util::IsValidIndex(s_brains, handle))
	{
		init_brain(handle);
	}
	return s_brains[handle];
}

//-------------------------------------------------------------------------------------------------
// Player pathfinding bot

bool try_player_pathfind(Vec3 goal)
{
	Brain& brain = get_brain(0);

	Pathfind::AstarParam param
	{
		.max_cost = 100, // we'll see if this is enough
		.ignore_creatures = true,
		.allow_unexplored = false,
	};
	Pathfind::astar(Player::pos(), goal, param, brain.move_stack);

	return !brain.move_stack.empty();
}

bool try_player_collect()
{
	Brain& brain = get_brain(0);

	Pathfind::ExploreParam param
	{
		.max_cost = Player::vision_radius,
		.allow_stairs = false,
		.goal = Pathfind::ExploreParam::GoalType::Item,
	};
	Pathfind::into_darkness(Player::pos(), param, brain.move_stack);

	return !brain.move_stack.empty();
}

bool try_player_explore()
{
	Brain& brain = get_brain(0);

	Pathfind::ExploreParam param
	{
		.max_cost = 100,
		.allow_stairs = false,
		.goal = Pathfind::ExploreParam::GoalType::Darkness,
	};
	Pathfind::into_darkness(Player::pos(), param, brain.move_stack);

	return !brain.move_stack.empty();
}


Vec2 try_get_next_player_move()
{
	Brain& brain = get_brain(0);
	if (!brain.move_stack.empty())
	{
		Vec3 const next_pos = Util::PopBack(brain.move_stack);
		Vec2 const next_move = next_pos.xy() - Player::pos().xy();
		if (next_move.x >= -1 && next_move.x <= 1 && next_move.y >= -1 && next_move.y <= 1)
		{
			return next_move;
		}
	}
	return {0,0};
}

void clear_player_path()
{
	Brain& brain = get_brain(0);
	brain.move_stack.clear();
}

//-------------------------------------------------------------------------------------------------
// Enemy creature bot

void do_turn (Creature::Handle creature)
{
	Brain& brain = s_brains[creature];
	Thoughts thoughts {};

	check_for_target(creature, brain, thoughts);

	check_transitions(creature, brain, thoughts);

	switch(brain.state)
	{
		case Bot::Rest: 
			creature.rest_step();
			break;

		case Bot::Blunder:
			bot_blunder(creature, brain, thoughts);
			break;

		case Bot::Chase:
			bot_chase(creature, brain, thoughts);
			break;

		case Bot::Fight:
			bot_fight(creature, brain, thoughts);
			break;
	}
}

// ------------------------------------------------------------------------------------------------
// Main state functions

void check_for_target(Creature::Handle creature, Brain& brain, Thoughts& thoughts)
{
	// Todo: Could support other targets in future.
	brain.target = Creature::Player;

	World const& world = World::read();
	thoughts.target_line = world.get_los(creature.pos(), brain.target.pos(), c_CreatureVision);
	thoughts.target_visible = (thoughts.target_line != c_Invalid);

	if (thoughts.target_visible)
	{
		brain.target_pos = brain.target.pos();
	}
}

void check_transitions(Creature::Handle creature, Brain& brain, Thoughts& thoughts)
{
	if (thoughts.target_visible)
	{
		if (brain.state != State::Fight)
		{
			brain.state = State::Fight;
		}
	}
	else // target not visible
	{
		if (c_TerminatorMode) // hunt down player (for testing)
		{
			brain.state = State::Chase;
			brain.target_pos = Player::pos();
			return;
		}

		if (brain.state == State::Fight)
		{
			brain.state = State::Chase;
		}

		else if (brain.state == State::Chase)
		{
			if (brain.awareness <= 0)
			{
				if (Debug::enabled(Debug::Bot))
				{
					std::cout << std::format("{} - Awareness reached 0, stopping chase.\n",
						creature.short_name());
				}

				brain.state = State::Blunder;
				brain.patience = 0;
				brain.move_stack.clear();
			}
		}

		else if (brain.state == State::Blunder)
		{
			if (brain.patience <= 0 &&
				Random::one_in(creature.is_hurt() ? 2 : 4))
			{
				brain.state = State::Rest;
			}
		}

		else if (brain.state == State::Rest)
		{
			if (Random::one_in(40))
			{
				brain.state = State::Blunder;
			}
		}
	}
}

void bot_blunder(Creature::Handle creature, Brain& brain, Thoughts& thoughts)
{
	if (brain.patience <= 0)
	{
		// Pick a new target pos at random.
		int constexpr max_distance = 10;
		Box2 const box = Box2{creature.pos().xy(), Vec2{1,1}}.plus_border(max_distance);
		brain.target_pos = Random::in_box(box).xyz(creature.pos().z);

		brain.patience = manhattan_distance(creature.pos().xy(), brain.target_pos.xy());
	}

	bool moved = try_move_towards(creature, brain.target_pos);
	--brain.patience;

	if (!moved)
	{
		// Something's in the way.  Lose more patience.
		brain.patience -= 5;

		// And just rest for this turn.
		creature.rest_step();
	}

	if (creature.pos() == brain.target_pos)
	{
		// We made it!
		brain.patience = 0;
	}
}

void bot_chase(Creature::Handle creature, Brain& brain, Thoughts& thoughts)
{
	Vec3 const pos = creature.pos();
	bool moved = false;

	--brain.awareness;

	if (pos == brain.target_pos)
	{
		brain.move_stack.clear();

		if (brain.target.valid())
		{
			// Find the target by, er... intuition.
			brain.target_pos = brain.target.pos();
		}
		else
		{
			brain.awareness = 0;
			return;
		}
	}
	else if (pos.z == brain.target_pos.z
		&& chessboard_distance(pos.xy(), brain.target_pos.xy()) == 1)
	{
		// Only one square away, you can do it!
		moved = try_move_towards(creature, brain.target_pos);
	}

	if (!moved && !brain.move_stack.empty())
	{
		// We have a plan.  See if we can still follow it.
		moved = try_follow_path(creature, brain.move_stack);
	}

	if (!moved && Game::get_turn_number() >= brain.pathfind_ready)
	{
		// Try to formulate a new plan.
		Pathfind::AstarParam param
		{
			.max_cost = c_MaxPathCost,
			.ignore_creatures = false
		};
		Pathfind::astar(pos, brain.target_pos, param, brain.move_stack);

		// Cooldown before pathfinding again.
		brain.pathfind_ready = Game::get_turn_number() + c_PathfindCooldown;

		if (!brain.move_stack.empty())
		{
			if (Debug::enabled(Debug::Bot))
			{
				std::cout << std::format("{} - Found path with length {}.\n",
					creature.short_name(), brain.move_stack.size());
			}

			moved = try_follow_path(creature, brain.move_stack);
		}
		else
		{
			if (Debug::enabled(Debug::Bot))
			{
				std::cout << std::format("{} - Pathfinding failed.\n", creature.short_name());
			}
		}
	}

	if (!moved)
	{
		moved = try_move_towards(creature, brain.target_pos);
	}

	if (!moved)
	{
		creature.rest_step();
	}
}

void bot_fight(Creature::Handle creature, Brain& brain, Thoughts& thoughts)
{
	if (!is_aware(creature))
	{
		// Spend a turn noticing the player.
		brain.awareness = c_MaxAwareness;
		Draw::creature_message(creature, Grammar::You(creature) + " sees you!");

		// And break automove since player probably wants to respond to this.
		Player::stop_automove();

		return;
	}

	if (creature.num_spells() > 0)
	{
		Spell::Index spell = choose_spell(creature, Player::handle());
		if (within_range(creature.pos(), Player::pos(), Spell::get_range(spell)))
		{
			try_cast_spell(spell, creature, Player::pos(), thoughts.target_line);
		}
		else
		{
			bool moved = try_move_towards(creature, Player::pos());
			if (!moved)
			{
				creature.rest_step();
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
// Other helpers

bool is_aware(Creature::Handle const creature)
{
	return s_brains[creature].awareness > 0;
}

// A naïve move straight towards the destination.
bool try_move_towards(Creature::Handle creature, Vec3 dest)
{
	if (creature.pos() == dest)
	{
		return false;
	}

	Vec3 const to_dest = dest - creature.pos();
	Vec2 const move_dir = {
		Math::Sign(to_dest.x),
		Math::Sign(to_dest.y)
	};

	bool moved = try_move(creature, move_dir, MoveMode::Walk);

	if (!moved)
	{
		// Try alternate routes.
		CompassDirection dir = to_compass(move_dir);
		Vec2 alt0 = c_Compass[get_clockwise(dir)];
		Vec2 alt1 = c_Compass[get_counterclockwise(dir)];

		if (Random::coinflip())
		{
			std::swap(alt0,alt1);
		}

		moved = try_move(creature, alt0, MoveMode::Walk);
		if (!moved)
		{
			moved = try_move(creature, alt1, MoveMode::Walk);
		}
	}

	return moved;
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
	Spell::TempList spell_list = caster.spells_known();

	// 1/3 chance of doing attack with best predicted damage
	if (Random::one_in(3))
	{
		spell_chosen = highest_predicted_damage_spell(caster, target, spell_list);
		if (spell_chosen != Spell::None)
		{
			if (Debug::enabled(Debug::Bot))
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
			if (Debug::enabled(Debug::Bot))
			{
				std::cout << std::format("{} using random reasonable spell = {}\n",
					caster.short_name(), Spell::get_name(spell_chosen));
			}

			return spell_chosen;
		}
	}

	if (Debug::enabled(Debug::Bot))
	{
		std::cout << std::format("{} couldn't find good spell.  Using random = {}\n",
			caster.short_name(), Spell::get_name(spell_chosen));
	}

	// if can't find anything that might succeed, give up and just cast whatever
	return spell_chosen;
}

Spell::Index highest_predicted_damage_spell (Creature::Handle caster, Creature::Handle target,
	Spell::TempList const & spell_list)
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
	if (spell == Spell::FiniteIncantatem)
	{
		if ( !caster.has_status(Status::Batty)
			//&& !caster.has_status(Status::Confused)
			//&& !caster.has_status(Status::Water)
			&& !caster.has_status(Status::Dancing)
			&& !caster.has_status(Status::Impeded)
			&& !caster.has_status(Status::LegLocked)
			&& !caster.has_status(Status::Tickled)
			&& !caster.has_status(Status::TongueTied) )
		{
			return true;
		}
	}

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
