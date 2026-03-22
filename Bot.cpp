#include "Bot.h"

#include "Ability.h"
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
#include "PerfTimer.h"
#include "Player.h"
#include "Random.h"
#include "Serialize.h"
#include "Spell.h"
#include "Stairs.h"
#include "Status.h"
#include "Taunt.h"
#include "Terrain.h"
#include "VectorUtil.h"
#include "World.h"

#include <array>
#include <format>

namespace Bot
{

static bool constexpr c_TerminatorMode = false;

// Number of turns a bot will remain "aware" after losing sight of player.
static int constexpr c_MaxAwareness = 15;

// Maximum path length to consider when using a-star pathfinding.
static int constexpr c_MaxPathCost = 25;

// After using the pathfinder, AI isn't allowed to use it again for this many turns.
// For performance reasons, and so they don't change directions wildly.
static int constexpr c_PathfindCooldown = 5;

static int constexpr c_CohesionDist = 4;

using MoveStack = std::vector<Vec3>;

static std::array<Brain, Creature::c_MaxCreatures> s_brains;
static std::array<MoveStack, Creature::c_MaxCreatures> s_move_stacks;

// Data used by bot during a single turn.
struct Thoughts
{
	bool target_visible = false;
	int target_line = c_Invalid;

	bool clear_line_of_fire = false;

	// Set to true if we try to use an ability from out of range.
	bool want_to_approach = false;

	bool has_taunted = false;
};

struct AttackOption
{
	enum Type : byte
	{
		None,
		Spell,
		Ability
	};

	Type type = None;
	int index = c_Invalid;
};
using AttackTempList = std::vector<AttackOption, Scratch<AttackOption>>;

// ------------------------------------------------------------------------------------------------
// Helper function declarations

Brain& get_brain(Creature::Handle handle);
std::vector<Vec3>& get_move_stack(Creature::Handle handle);

char const* state_name(Bot::State state);

void update_attack_ranges(Creature::Handle creature, Brain& brain);
void check_for_target(Creature::Handle creature, Brain& brain, Thoughts& thoughts);
void check_transitions(Creature::Handle creature, Brain& brain, Thoughts& thoughts);
void enter_state(Creature::Handle creature, Brain& brain, Bot::State state);

void bot_blunder(Creature::Handle creature, Brain& brain, Thoughts& thoughts);
void bot_regroup(Creature::Handle creature, Brain& brain, Thoughts& thoughts);
void bot_investigate(Creature::Handle creature, Brain& brain, Thoughts& thoughts);
void bot_chase(Creature::Handle creature, Brain& brain, Thoughts& thoughts);
void bot_fight(Creature::Handle creature, Brain& brain, Thoughts& thoughts);

bool is_aware(Creature::Handle const creature);
bool is_separated_from_leader(Creature::Handle const creature);
bool has_clear_line_of_fire(Creature::Handle const creature, Brain const& brain, Thoughts const& thoughts);

bool try_move (Creature::Handle creature, Vec2 relative_move);
bool try_move_towards(Creature::Handle creature, Vec3 dest);
bool try_follow_path(Creature::Handle creature, std::vector<Vec3>& move_stack);
bool try_sidestep(Creature::Handle creature, int target_line);
bool try_go_to_firing_position(Creature::Handle creature, Brain& brain);
bool try_go_to_target_pos(Creature::Handle creature, Brain& brain);

bool try_attack(Creature::Handle creature, Brain& brain, Thoughts& thoughts);
AttackOption choose_attack(Creature::Handle creature, Creature::Handle target);

void use_spell(Creature::Handle creature, Brain& brain, Thoughts& thoughts, Spell::Index spell);
float rate_spell (Creature::Handle creature, Creature::Handle target, Spell::Index spell);
bool spell_is_useless (Spell::Index spell, Creature::Handle caster, Creature::Handle target);
Vec3 aim_halfway_between (Creature::Handle caster, Creature::Handle target,
	int line_id, int spell_range);

void use_ability(Creature::Handle creature, Brain& brain, Thoughts& thoughts, Ability::Index ability);
float rate_ability (Creature::Handle creature, Creature::Handle target, Ability::Index ability);

void taunt_greeting(Creature::Handle creature, Brain& brain, Thoughts& thoughts);
void taunt_followup(Creature::Handle creature, Brain& brain, Thoughts& thoughts);
void taunt_fight(Creature::Handle creature, Brain& brain, Thoughts& thoughts);
void taunt_attack_spell(Creature::Handle creature, Brain& brain, Thoughts& thoughts,
	Spell::Index spell);

// ------------------------------------------------------------------------------------------------
// Interface functions

void clear()
{
	for (Brain& brain : s_brains)
	{
		brain = Brain{};
	}

	for (std::vector<Vec3>& row : s_move_stacks)
	{
		row.clear();
	}
}

void serialize(ISerializer& s)
{
	int num_brains = Creature::c_MaxCreatures;
	s.srz_int(num_brains);

	if (num_brains != Creature::c_MaxCreatures)
	{
		DebugBreak("Array size mismatch!");
		return;
	}

	s.srz_array_data(s_brains.data(), Creature::c_MaxCreatures);

	for (int i = 0; i < Creature::c_MaxCreatures; ++i)
	{
		s.srz_vector(s_move_stacks[i], "move stack");
	}
}

void reset_brain(Creature::Handle handle)
{
	// Technically we leave a brain for the player.
	// It's not worth the confusingness of offsetting the indices.
	// We even use the move stack a little for auto-move.

	s_brains.at((int)handle) = Brain{};
	s_move_stacks.at((int)handle).clear();
}

// ------------------------------------------------------------------------------------------------
// Essential access functions

Brain& get_brain(Creature::Handle handle)
{
	return s_brains.at(handle);
}

std::vector<Vec3>& get_move_stack(Creature::Handle handle)
{
	return s_move_stacks.at(handle);
}

//-------------------------------------------------------------------------------------------------
// Player pathfinding bot

bool try_player_pathfind(Vec3 goal)
{
	MoveStack& move_stack = get_move_stack(0);

	Pathfind::AstarParam param
	{
		.max_cost = 100, // we'll see if this is enough
		.ignore_creatures = true,
		.allow_unexplored = false,
	};
	Pathfind::astar(Player::pos(), goal, param, move_stack);

	return !move_stack.empty();
}

bool try_player_collect()
{
	MoveStack& move_stack = get_move_stack(0);

	Pathfind::ExploreParam param
	{
		.max_cost = Player::c_VisionRadius,
		.allow_stairs = false,
		.goal = Pathfind::ExploreParam::GoalType::Item,
	};
	Pathfind::into_darkness(Player::pos(), param, move_stack);

	return !move_stack.empty();
}

bool try_player_explore()
{
	MoveStack& move_stack = get_move_stack(0);

	Pathfind::ExploreParam param
	{
		.max_cost = 100,
		.allow_stairs = false,
		.goal = Pathfind::ExploreParam::GoalType::Darkness,
	};
	Pathfind::into_darkness(Player::pos(), param, move_stack);

	return !move_stack.empty();
}


Vec2 pop_player_path()
{
	MoveStack& move_stack = get_move_stack(0);

	if (!move_stack.empty())
	{
		Vec3 const next_pos = Util::PopBack(move_stack);
		Vec2 const next_move = next_pos.xy() - Player::pos().xy();
		if (next_move.x >= -1 && next_move.x <= 1 && next_move.y >= -1 && next_move.y <= 1)
		{
			return next_move;
		}
	}
	return {0,0};
}

bool has_player_path()
{
	MoveStack& move_stack = get_move_stack(0);
	return !move_stack.empty();
}

void clear_player_path()
{
	MoveStack& move_stack = get_move_stack(0);
	move_stack.clear();
}

//-------------------------------------------------------------------------------------------------
// Enemy creature bot

void do_turn (Creature::Handle creature)
{
	PerfTimer perf("Bot::do_turn");

	Brain& brain = s_brains[creature];
	Thoughts thoughts {};

	update_attack_ranges(creature, brain);

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

		case Bot::Regroup:
			bot_regroup(creature, brain, thoughts);
			break;

		case Bot::Investigate:
			bot_investigate(creature, brain, thoughts);
			break;

		case Bot::Chase:
			bot_chase(creature, brain, thoughts);
			break;

		case Bot::Fight:
			bot_fight(creature, brain, thoughts);
			break;

		default:
			DebugBreak("Missing case in Bot::do_turn");
	}
}

void notify_investigate(Creature::Handle creature, Vec3 target_pos)
{
	Brain& brain = s_brains[creature];

	switch (brain.state)
	{
		case State::Rest:
		case State::Blunder:
		case State::Regroup:
		case State::Investigate:
			enter_state(creature, brain, State::Investigate);
			brain.target_pos = target_pos;
			brain.patience = manhattan_2d(creature.pos(), target_pos);
			break;

		case State::Chase:
			if (Creature::creature_at_pos(target_pos) == brain.target)
			{
				brain.awareness = c_MaxAwareness;
				brain.target_pos = target_pos;
			}
			break;

		case State::Fight:
			// Already in combat.
			break;

		default:
			DebugBreak("Missing state in notify_investigate");
	}
}

void notify_attacks_changed(Creature::Handle creature)
{
	Brain& brain = s_brains[creature];
	brain.any_attack_range = c_Invalid;
	brain.all_attack_range = c_Invalid;
	brain.dmg_attack_range = c_Invalid;
}

// ------------------------------------------------------------------------------------------------
// Main state functions

char const* state_name(Bot::State state)
{
	switch (state)
	{
		case State::Rest:			return "Rest";
		case State::Blunder:		return "Blunder";
		case State::Regroup:		return "Regroup";
		case State::Investigate:	return "Investigate";
		case State::Chase:			return "Chase";
		case State::Fight:			return "Fight";

		default:
			DebugBreak();
			return "ErrorState";
	}
}

void update_attack_ranges(Creature::Handle creature, Brain& brain)
{
	if (brain.any_attack_range == c_Invalid ||
		brain.all_attack_range == c_Invalid ||
		brain.dmg_attack_range == c_Invalid)
	{
		int most_damage = 0;

		brain.all_attack_range = creature.vision();
		brain.dmg_attack_range = -1;
		brain.any_attack_range = -1;

		for (Spell::Index spell : creature.spells_known())
		{
			if (Spell::get_target_type(spell) != Spell::TargetType::Self)
			{
				int const range = Spell::get_range(spell);
				brain.all_attack_range = std::min(range, brain.all_attack_range);
				brain.any_attack_range = std::max(range, brain.any_attack_range);

				int const dmg = Spell::get_damage(spell, creature);
				if (dmg >= most_damage)
				{
					brain.dmg_attack_range = std::max(range, brain.dmg_attack_range);
				}
			}
		}

		for (Ability::Index ability : creature.ability_list())
		{
			Ability::TargetType const target_type = Ability::target_type(ability);
			if (target_type != Ability::TargetType::Self)
			{
				int const range = Ability::get_range(ability);
				brain.all_attack_range = std::min(range, brain.all_attack_range);
				brain.any_attack_range = std::max(range, brain.any_attack_range);

				int const dmg = Ability::get_damage(ability);
				if (dmg >= most_damage)
				{
					brain.dmg_attack_range = std::max(range, brain.dmg_attack_range);
				}
			}
		}

		// Cases where we found NO attacks - don't bother trying to get close!
		if (brain.dmg_attack_range == -1)
		{
			brain.dmg_attack_range = creature.vision();
		}
		if (brain.any_attack_range == -1)
		{
			brain.any_attack_range = creature.vision();
		}
	}
}

void check_for_target(Creature::Handle creature, Brain& brain, Thoughts& thoughts)
{
	// Todo: Could support other targets in future.
	brain.target = Creature::Player;

	World const& world = World::read();
	thoughts.target_line = world.get_los(creature.pos(), brain.target.pos(), creature.vision());
	thoughts.target_visible = (thoughts.target_line != c_Invalid);

	if (thoughts.target_visible)
	{
		brain.target_pos = brain.target.pos();
		thoughts.clear_line_of_fire = has_clear_line_of_fire(creature, brain, thoughts);
	}
}

void check_transitions(Creature::Handle creature, Brain& brain, Thoughts& thoughts)
{
	if (thoughts.target_visible)
	{
		if (brain.state != State::Fight)
		{
			enter_state(creature, brain, State::Fight);
		}
	}
	else // target not visible
	{
		if (c_TerminatorMode) // hunt down player (for testing)
		{
			enter_state(creature, brain, State::Chase);
			brain.target_pos = Player::pos();
			return;
		}

		if (brain.state == State::Fight)
		{
			enter_state(creature, brain, State::Chase);
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

				enter_state(creature, brain, State::Blunder);
			}
		}

		else if (brain.state == State::Investigate)
		{
			if (brain.patience <= 0 || creature.pos() == brain.target_pos)
			{
				enter_state(creature, brain, State::Blunder);
			}
		}

		else if (brain.state == State::Regroup)
		{
			if (!creature.has_squad_leader())
			{
				enter_state(creature, brain, State::Rest);
			}
			if (get_move_stack(creature).empty() &&
				!is_separated_from_leader(creature))
			{
				enter_state(creature, brain, State::Rest);
			}
		}

		else if (brain.state == State::Blunder)
		{
			if (brain.patience <= 0 &&
				Random::one_in(creature.is_hurt() ? 2 : 4))
			{
				enter_state(creature, brain, State::Rest);
			}
		}

		else if (brain.state == State::Rest)
		{
			if (Random::one_in(5))
			{
				if (is_separated_from_leader(creature))
				{
					enter_state(creature, brain, State::Regroup);
				}

				else if (World::read().is_choke_point(creature.pos()) ||
					creature.has_tag(Creature::Tag::Bot_Blunder) ||
					Random::one_in(8))
				{
					enter_state(creature, brain, State::Blunder);
				}
			}
		}
	}
}

void enter_state(Creature::Handle creature, Brain& brain, Bot::State state)
{
	if (brain.state == state)
	{
		return;
	}

	brain.state = state;

	// Reset pathfinding when changing states.
	get_move_stack(creature).clear();
	brain.pathfind_ready = Game::get_turn_number();

	switch(state)
	{
		case State::Blunder:
		{
			brain.patience = 0;
		}
		break;
	}

	if (Debug::enabled(Debug::Bot))
	{
		std::cout << std::format("{} - Entering {} state.\n",
			creature.short_name(), state_name(state));
	}
}

void bot_blunder(Creature::Handle creature, Brain& brain, Thoughts& thoughts)
{
	if (brain.patience <= 0)
	{
		// Pick a new target pos at random.
		int constexpr max_distance = 10;
		Box2 const box = Box2::around_tile(creature.pos().xy(), max_distance);
		brain.target_pos = Random::in_box(box).xyz(creature.pos().z);

		brain.patience = manhattan(creature.pos().xy(), brain.target_pos.xy());
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

void bot_regroup(Creature::Handle creature, Brain& brain, Thoughts& thoughts)
{
	if (!creature.has_squad() || creature.is_squad_leader())
	{
		creature.rest_step();
		return;
	}

	Vec3 const pos = creature.pos();
	brain.target_pos = creature.squad_leader().pos();
	bool moved = false;

	if (pos.z == brain.target_pos.z
		&& chessboard(pos.xy(), brain.target_pos.xy()) <= 2)
	{
		get_move_stack(creature).clear();
		creature.rest_step();
		moved = true;
	}

	if (!moved)
	{
		moved = try_go_to_target_pos(creature, brain);
	}

	if (!moved)
	{
		creature.rest_step();
	}
}

void bot_investigate(Creature::Handle creature, Brain& brain, Thoughts& thoughts)
{
	bool moved = try_go_to_target_pos(creature, brain);
	--brain.patience;

	if (!moved)
	{
		// Something's in the way.  Lose more patience.
		brain.patience -= 3;

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
		get_move_stack(creature).clear();

		if (brain.target.valid())
		{
			// Find the target by, er... intuition?
			brain.target_pos = brain.target.pos();
		}
		else
		{
			brain.awareness = 0;
			return;
		}
	}

	moved = try_go_to_target_pos(creature, brain);

	if (!moved)
	{
		creature.rest_step();
	}
}

void bot_fight(Creature::Handle creature, Brain& brain, Thoughts& thoughts)
{
	PerfTimer perf0("bot_fight");

	// Notify squadmates over tactical radio.
	if (creature.has_squad())
	{
		Creature::HandleList const& squad_members = creature.squad_members();
		for (Creature::Handle const& ally : squad_members)
		{
			if (ally == creature)
			{
				continue;
			}

			notify_investigate(ally, brain.target_pos);
		}
	}

	if (!is_aware(creature))
	{
		// Spend a turn noticing the player.
		brain.awareness = c_MaxAwareness;

		taunt_greeting(creature, brain, thoughts);
		if (!thoughts.has_taunted)
		{
			Draw::creature_message(creature, Grammar::You(creature) + " sees you!");
		}

		return;
	}
	
	// Consider taunting the player.
	taunt_followup(creature, brain, thoughts);
	taunt_fight(creature, brain, thoughts);

	bool done = false;

	bool const any_range = range_2d(creature.pos(), brain.target_pos, brain.any_attack_range);
	bool const dmg_range = range_2d(creature.pos(), brain.target_pos, brain.dmg_attack_range);
	bool const all_range = range_2d(creature.pos(), brain.target_pos, brain.all_attack_range);

	bool const want_to_approach = !any_range ||
		(Random::coinflip() && !dmg_range) ||
		(Random::coinflip() && !all_range);

	if (want_to_approach)
	{
		done = try_move_towards(creature, Player::pos());
	}

	if (!done && !thoughts.clear_line_of_fire && !Random::one_in(3))
	{
		done = try_go_to_firing_position(creature, brain);
	}

	// Squad creatures - Try not to block the hallway
	if (!done && creature.has_squad() && World::read().is_choke_point(creature.pos()) &&
		Random::coinflip())
	{
		done = try_move_towards(creature, Player::pos());
	}

	if (!done)
	{
		done = try_attack(creature, brain, thoughts);
	}

	if (!done && creature.has_tag(Creature::Tag::Bot_Sidestep))
	{
		done = try_sidestep(creature, thoughts.target_line);
	}

	if (!done)
	{
		creature.rest_step();
	}
}

//-------------------------------------------------------------------------------------------------
// Other helpers

bool is_aware(Creature::Handle const creature)
{
	return s_brains[creature].awareness > 0;
}

bool is_separated_from_leader(Creature::Handle const creature)
{
	if (!creature.has_squad_leader())
	{
		return false;
	}

	Vec3 const pos = creature.pos();
	Vec3 const leader_pos = creature.squad_leader().pos();

	return leader_pos.z != pos.z || !range_2d(pos, leader_pos, c_CohesionDist);
}

bool has_clear_line_of_fire(Creature::Handle const creature, Brain const& brain,
	Thoughts const& thoughts)
{
	if (!thoughts.target_visible)
	{
		return false;
	}

	return Action::is_line_of_fire_clear(creature, creature.pos(), brain.target_pos,
		thoughts.target_line);
}

// Move but not if it's hazardous.
bool try_move (Creature::Handle creature, Vec2 relative_move)
{
	// Ignore hazards if current pos is already hazardous.
	bool const ignore_hazards = creature.finds_pos_hazardous(creature.pos());

	if (Action::is_move_hazardous(creature, relative_move))
	{
		return false;
	}

	if (!creature.ready_to_move())
	{
		creature.rest_step();
		return true;
	}

	return Action::try_move(creature, relative_move, MoveMode::Walk);
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

	bool moved = Bot::try_move(creature, move_dir);

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

		moved = Bot::try_move(creature, alt0);
		if (!moved)
		{
			moved = Bot::try_move(creature, alt1);
		}
	}

	return moved;
}



bool try_follow_path(Creature::Handle creature, std::vector<Vec3>& move_stack)
{
	// Early exit here so we don't consume the move from the stack.
	if (!creature.ready_to_move())
	{
		creature.rest_step();
		return true;
	}

	// Try to follow our plan
	Vec3 const next_pos = Util::PopBack(move_stack);
	Vec2 const next_move = next_pos.xy() - creature.pos().xy();
	if (next_move.x >= -1 && next_move.x <= 1 && next_move.y >= -1 && next_move.y <= 1)
	{
		return Bot::try_move(creature, next_move);
	}

	return false;
}

bool try_sidestep(Creature::Handle creature, int target_line)
{
	Vec2 const v0 = LineCache::read_line(target_line, 1);
	CompassDirection const forwards = to_compass(v0);
	CompassDirection const left = get_counterclockwise_90(forwards);
	CompassDirection const right = get_clockwise_90(forwards);
	Vec2 v1 = c_Compass[left];
	Vec2 v2 = c_Compass[right];

	if (Random::coinflip())
	{
		std::swap(v1,v2);
	}

	bool moved = Bot::try_move(creature, v1);

	if (!moved)
	{
		moved = Bot::try_move(creature, v2);
	}

	return moved;
}


bool try_go_to_firing_position(Creature::Handle creature, Brain& brain)
{
	MoveStack& move_stack = get_move_stack(creature);
	bool moved = false;

	// If we already have a solid plan, execute on that.
	if (!moved && !move_stack.empty() &&
		Action::is_clear_firing_position(creature, move_stack[0], brain.target_pos,
			brain.dmg_attack_range))
	{
		moved = try_follow_path(creature, move_stack);
	}

	// If not, try to formulate a new plan.
	if (!moved && Game::get_turn_number() >= brain.pathfind_ready)
	{
		Pathfind::FiringPositionParams param
		{
			.max_cost = creature.vision(),
			.max_range = brain.dmg_attack_range,
		};
		Pathfind::find_firing_position(creature, brain.target_pos, param, move_stack);

		// Cooldown before pathfinding again.
		brain.pathfind_ready = Game::get_turn_number() + c_PathfindCooldown;

		if (!move_stack.empty())
		{
			if (Debug::enabled(Debug::Bot))
			{
				std::cout << std::format("{} - Found path to firing position, length {}.\n",
					creature.short_name(), move_stack.size());
			}

			moved = try_follow_path(creature, move_stack);
		}
		else if (Debug::enabled(Debug::Bot))
		{
			std::cout << std::format("{} - Failed to find firing position.\n", creature.short_name());
		}
	}

	return moved;
}

// Try pathfinding if possible, or otherwise just move towards.
bool try_go_to_target_pos(Creature::Handle creature, Brain& brain)
{
	MoveStack& move_stack = get_move_stack(creature);

	Vec3 const pos = creature.pos();
	bool moved = false;

	if (pos.z == brain.target_pos.z
		&& chessboard(pos.xy(), brain.target_pos.xy()) == 1)
	{
		// Only one square away, you can do it!
		moved = try_move_towards(creature, brain.target_pos);
	}

	if (!moved && !move_stack.empty())
	{
		// We have a plan.  See if we can still follow it.
		moved = try_follow_path(creature, move_stack);
	}

	if (!moved && Game::get_turn_number() >= brain.pathfind_ready)
	{
		// Try to formulate a new plan.
		Pathfind::AstarParam param
		{
			.max_cost = c_MaxPathCost,
			.ignore_creatures = false
		};
		Pathfind::astar(pos, brain.target_pos, param, move_stack);

		// Cooldown before pathfinding again.
		brain.pathfind_ready = Game::get_turn_number() + c_PathfindCooldown;

		if (!move_stack.empty())
		{
			if (Debug::enabled(Debug::Bot))
			{
				std::cout << std::format("{} - Found path with length {}.\n",
					creature.short_name(), move_stack.size());
			}

			moved = try_follow_path(creature, move_stack);
		}
		else if (Debug::enabled(Debug::Bot))
		{
			std::cout << std::format("{} - Pathfinding failed.\n", creature.short_name());
		}
	}

	if (!moved)
	{
		moved = try_move_towards(creature, brain.target_pos);
	}

	return moved;
}

bool try_attack(Creature::Handle creature, Brain& brain, Thoughts& thoughts)
{
	PerfTimer perf0("try_attack");

	AttackOption const attack = choose_attack(creature, brain.target);

	if (attack.type == AttackOption::Spell)
	{
		use_spell(creature, brain, thoughts, (Spell::Index)attack.index);
		return true;
	}
	else if (attack.type == AttackOption::Ability)
	{
		use_ability(creature, brain, thoughts, (Ability::Index)attack.index);
		return true;
	}
	else // None
	{
		return false;
	}
}

AttackOption choose_attack (Creature::Handle creature, Creature::Handle target)
{
	PerfTimer perf0("choose_attack");

	AttackTempList options;
	FloatTempList weights;
	options.reserve(creature.num_spells() + creature.num_abilities());
	weights.reserve(creature.num_spells() + creature.num_abilities());

	float total_weight = 0.0f;

	for (Ability::Index ability : creature.ability_list())
	{
		float const rating = rate_ability(creature, target, ability);
		if (rating > 0.0f)
		{
			options.emplace_back(AttackOption::Ability, (int)ability);
			weights.push_back(rating);
			total_weight += rating;
		}
	}

	for (Spell::Index spell : creature.spells_known())
	{
		float const rating = rate_spell(creature, target, spell);
		if (rating > 0.0f)
		{
			options.emplace_back(AttackOption::Spell, (int)spell);
			weights.push_back(rating);
			total_weight += rating;
		}
	}

	if (total_weight > 0.0f)
	{
		int const choice = Random::weighted_index(weights);
		return options.at(choice);
	}
	else
	{
		return AttackOption{};
	}
}

void use_spell (Creature::Handle creature, Brain& brain, Thoughts& thoughts, Spell::Index spell)
{
	PerfTimer perf0("use_spell");

	if (Spell::get_target_type(spell) == Spell::TargetType::Self)
	{
		Action::try_cast_spell(spell, creature, creature.pos(), c_Invalid);
	}
	else
	{
		assert(brain.target.valid());

		if (spell == Spell::Fumos)
		{
			Vec3 aim_pos = aim_halfway_between(creature, brain.target, thoughts.target_line,
				Spell::get_range(spell));
			int const aim_line = (aim_pos == creature.pos()) ? c_Invalid : thoughts.target_line;
			Action::try_cast_spell(spell, creature, aim_pos, aim_line);
		}
		else
		{
			assert(Spell::in_range(spell, creature.pos(), brain.target_pos));
			taunt_attack_spell(creature, brain, thoughts, spell);
			Action::try_cast_spell(spell, creature, brain.target_pos, thoughts.target_line);
		}
	}
}

float rate_spell (Creature::Handle caster, Creature::Handle target, Spell::Index spell)
{
	if (!Spell::in_range(spell, caster.pos(), target.pos()))
	{
		return 0.0f;
	}

	if (spell_is_useless(spell, caster, target))
	{
		return 0.0f;
	}

	float rating = 0.0f;

	switch (spell)
	{
		// Special cases:

		case Spell::Flipendo:
			// It's the main attacking spell in the early game, so use it!
			rating = 3.5f;
			break;

		case Spell::Accio:
			rating = 2.0f;
			break;

		default:
			if (Spell::is_damaging(spell))
			{
				rating = (float)Spell::get_damage(spell, caster);
			}
			else
			{
				// Estimate based on the difficulty, I suppose.
				rating = Spell::get_difficulty(spell) * 0.1f;
			}
	}

	// Factor in accuracy.
	if (Spell::has_accuracy(spell))
	{
		int const accuracy = Beam::accuracy_at_range(Spell::get_accuracy(spell),
			caster.pos(), target.pos());
		rating = rating * (accuracy / 100.0f);
	}

	// Factor in miscast rate.
	float const success_rate = 100.0f - caster.miscast_rate_for_spell(spell);
	rating = rating * (success_rate / 100.0f);

	return rating;
}

bool spell_is_useless (Spell::Index spell, Creature::Handle caster, Creature::Handle target)
{
	// if we have virtually no chance of casting it, it's useless
	float miscast_rate = caster.miscast_rate_for_spell(spell);
	if (miscast_rate >= 95.0)
		return true;

	// Non-combat spells
	if (spell == Spell::Alohomora)
	{
		return true;
	}

	// Although not technically useless, don't re-apply statuses already at high levels.
	if ((spell == Spell::Tarantallegra && target.status_severity(Status::Dancing) > 7) ||
		(spell == Spell::LocomotorMortis && target.status_severity(Status::LegLocked) > 7) ||
		(spell == Spell::LacarnumInflamare && target.status_severity(Status::Burning) > 7) ||
		(spell == Spell::Rictusempra && target.status_severity(Status::Tickled) > 7) ||
		(spell == Spell::Mimblewimble && target.status_severity(Status::TongueTied) > 7) ||
		(spell == Spell::Impedementa && target.status_severity(Status::Impeded) > 7) ||
		(spell == Spell::BatBogey && target.status_severity(Status::Batty) > 7) )
	{
		return true;
	}
	
	// Finite is useless if there's no enchantment to break
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

Vec3 aim_halfway_between (Creature::Handle caster, Creature::Handle target,
	int line_id, int spell_range)
{
	// Aim halfway between us
	float const current_range = euclid_2d(caster.pos(), target.pos());
	float const half_range = current_range/2.0f;
	Vec3 aim_pos = caster.pos();
	if (line_id != LineCache::c_StairsLine)
	{
		LineCache::Itr3D itr(aim_pos, line_id);
		while (itr)
		{
			++itr;
			if (range_2d(caster.pos(), *itr, (int)half_range) &&
				range_2d(caster.pos(), *itr, spell_range))
			{
				aim_pos = *itr;
			}
			else
			{
				break;
			}
		}
	}
	return aim_pos;
}

void use_ability(Creature::Handle creature, Brain& brain, Thoughts& thoughts, Ability::Index ability)
{
	PerfTimer perf0("use_ability");

	Ability::TargetType const target_type = Ability::target_type(ability);
	if (target_type == Ability::TargetType::Self)
	{
		Action::try_use_ability(ability, creature, creature.pos(), c_Invalid);
	}
	else
	{
		assert(Ability::in_range(ability, creature.pos(), brain.target_pos));
		Action::try_use_ability(ability, creature, brain.target_pos, thoughts.target_line);
	}
}

float rate_ability(Creature::Handle creature, Creature::Handle target, Ability::Index ability)
{
	if (Ability::is_in_cooldown(creature, ability))
	{
		return 0.0f;
	}

	if (!Ability::in_range(ability, creature.pos(), target.pos()))
	{
		return 0.0f;
	}

	float rating = 0.0f;

	switch (ability)
	{
		case Ability::EatBean:
			rating = (creature.has_item() && creature.peek_item().type() == Item::BBBean) ?
				5.0f : // Greatsome joy and felicitation!  Gurgi love eat bean!
				0.0f;
			break;

		case Ability::TripKick:
			rating = 5.0f;
			break;

		case Ability::Believe:
			return 10.0f * (1.0f - creature.hp_percent());
			break;

		default:
			if (Ability::is_damaging(ability))
			{
				rating = (float)Ability::get_damage(ability); 
			}
			else
			{
				// Who knows, really?
				rating = 1.0f;
			}
	}

	// Factor in accuracy.
	if (Ability::has_accuracy(ability))
	{
		int const accuracy = Beam::accuracy_at_range(Ability::get_accuracy(ability),
			creature.pos(), target.pos());
		rating = rating * (accuracy / 100.0f);
	}

	return rating;
}

void say_taunt(Creature::Handle creature, Brain& brain, Thoughts& thoughts, int taunt)
{
	Taunt::say_taunt(creature, taunt);
	brain.last_taunt = taunt;
	thoughts.has_taunted = true;
}

void taunt_greeting(Creature::Handle creature, Brain& brain, Thoughts& thoughts)
{
	if (!thoughts.has_taunted && brain.target == Player::handle() && creature.visible())
	{
		IntTempList taunts;
		Taunt::find_taunts(creature, Taunt::Greeting, c_Invalid, taunts);

		int const num_taunts = Util::Size(taunts);
		if (num_taunts > 0 && Random::coinflip())
		{
			say_taunt(creature, brain, thoughts, Random::from_vector(taunts));
		}
	}
}

void taunt_followup(Creature::Handle creature, Brain& brain, Thoughts& thoughts)
{
	if (!thoughts.has_taunted && brain.last_taunt != c_Invalid)
	{
		int const taunt = Taunt::find_followup(creature, brain.last_taunt);
		if (taunt != c_Invalid)
		{
			say_taunt(creature, brain, thoughts, taunt);
		}
	}
}

void taunt_fight(Creature::Handle creature, Brain& brain, Thoughts& thoughts)
{
	PerfTimer perf("taunt_fight");

	if (!thoughts.has_taunted && brain.target == Player::handle() && creature.visible())
	{
		IntTempList taunts;
		Taunt::find_taunts(creature, Taunt::AnyTime, c_Invalid, taunts);

		if (creature.hp_percent() <= 0.5f &&
			creature.hp_percent() < brain.target.hp_percent())
		{
			Taunt::find_taunts(creature, Taunt::Losing, c_Invalid, taunts);
		}
		else if (brain.target.hp_percent() < 0.6f && 
			brain.target.hp_percent() < creature.hp_percent())
		{
			Taunt::find_taunts(creature, Taunt::Winning, c_Invalid, taunts);
		}

		Spell::Index const player_miscast = Player::get_recent_miscast();
		if (player_miscast != Spell::None)
		{
			Taunt::find_taunts(creature, Taunt::PlayerMiscast, player_miscast, taunts);
		}

		Taunt::find_status_taunts(creature, brain.target, taunts);

		int const num_taunts = Util::Size(taunts);
		if (num_taunts > 0)
		{
			// More likely to taunt if we have more available
			float const p_taunt = std::min(3.0f, sqrt((float)num_taunts));
			if (Random::in_range(0.0f, 8.0f) < p_taunt)
			{
				say_taunt(creature, brain, thoughts, Random::from_vector(taunts));
			}
		}
	}
}

void taunt_attack_spell(Creature::Handle creature, Brain& brain, Thoughts& thoughts,
	Spell::Index spell)
{
	if (!thoughts.has_taunted && brain.target == Player::handle() && creature.visible())
	{
		IntTempList taunts;
		Taunt::find_taunts(creature, Taunt::AttackSpell, spell, taunts);

		int const num_taunts = Util::Size(taunts);
		if (num_taunts > 0)
		{
			// More likely to taunt if we have more available
			float const p_taunt = std::min(3.0f, sqrt((float)num_taunts));
			if (Random::in_range(0.0f, 8.0f) < p_taunt)
			{
				say_taunt(creature, brain, thoughts, Random::from_vector(taunts));
			}
		}
	}
}

} // namespace Bot
