#include "Ability.h"
#include "Beam.h"
#include "BitFlag.h"
#include "Bot.h"
#include "Colour.h"
#include "Cloud.h"
#include "Creature.h"
#include "Debug.h"
#include "Draw.h"
#include "Grammar.h"
#include "Random.h"
#include "Spell.h"
#include "SpellEffect.h"
#include "Stairs.h"
#include "Terrain.h"
#include "World.h"

#include <cassert>
#include <format>

namespace Beam
{

int constexpr c_AccuracyLoss = 3; // per square
int constexpr c_MinRangedAccuracy = 40; // before character stats are applied

// ------------------------------------------------------------------------------------------------
// helper function declarations

static Beam::Data make_spell_beam (Spell::Index, Creature::Handle caster, Vec3 target_pos,
	bool caster_aimed, int line_id);
static Beam::Data make_ability_beam (Ability::Index, Creature::Handle caster, Vec3 target_pos,
	int line_id);
static void shoot_beam (Beam::Data & beam);
static void shoot_beam_on_line (Beam::Data & beam);
static void shoot_beam_on_stairs (Beam::Data & beam);
static void sweep_beam_on_current_pos (Beam::Data & beam, Draw::View& view, int codepoint,
	char const* colour, LineCache::Itr3D const& line_itr);
static void test_for_impact (Beam::Data & beam, LineCache::Itr3D const & line_itr);
static int get_hit_chance (Beam::Data const & beam, Creature::Handle target);
static void hit_creature (Beam::Data const & beam, Creature::Handle target,
	LineCache::Itr3D const & line);
static void detonate_in_midair (Beam::Data const & beam, LineCache::Itr3D const & line);

// ------------------------------------------------------------------------------------------------
// interface function implementations

void shoot_spell (Spell::Index spell, Creature::Handle caster, Vec3 target_pos,
	bool caster_aimed, int line_id)
{
	Beam::Data beam = make_spell_beam(spell, caster, target_pos, caster_aimed, line_id);
	shoot_beam(beam);
}

void shoot_ability (Ability::Index ability, Creature::Handle user, Vec3 target_pos,
	int line_id)
{
	Beam::Data beam = make_ability_beam(ability, user, target_pos, line_id);
	shoot_beam(beam);
}

int accuracy_at_range(int base_accuracy, Vec3 start, Vec3 end)
{
	int const dist = (int)euclidean_distance(start, end);
	int const loss = dist * c_AccuracyLoss;
	return std::max(c_MinRangedAccuracy, base_accuracy - loss);
}

// ------------------------------------------------------------------------------------------------
// helper function implementations

Beam::Data make_spell_beam (Spell::Index spell, Creature::Handle caster, Vec3 target_pos,
	bool caster_aimed, int line_id)
{
	assert(caster.pos() != target_pos); // Should not be shooting beam with zero trajectory.
	assert(line_id != c_Invalid);

	Creature::Handle intended_target = Creature::None;
	if (caster_aimed)
	{
		intended_target = Creature::creature_at_pos(target_pos);
	}

	int const spell_range = Spell::get_range(spell);
	const World& world = World::read();

	uint flags = f_None;
	if (Spell::get_target_type(spell) == Spell::TargetType::Tile)
	{
		Util::SetFlag(flags, f_StopOnTarget);
	}
	if (caster_aimed)
	{
		Util::SetFlag(flags, f_CasterAimed);
	}

	return Beam::Data
	{
		.start_pos = caster.pos(),
		.target_pos = target_pos,
		.pos = caster.pos(),
		.type = Beam::Type::Spell,
		.effect_func = Spell::get_effect_func(spell),
		.noun = "spell",
		.colour = Spell::get_colour(spell),
		.caster = caster,
		.intended_target = intended_target,
		.codepoint = Spell::get_name(spell).at(0),
		.trajectory = line_id,
		.max_range = spell_range,
		.base_accuracy = Spell::get_accuracy(spell),
		.cloud_accuracy_loss = 0,
		.damage_type = Spell::damage_type(spell),
		.damage = Spell::get_damage(spell, caster),
		.spell_power = Spell::get_power(spell, caster),
		.flags = flags,
		.done = false
	};
}

Beam::Data make_ability_beam (Ability::Index ability, Creature::Handle caster, Vec3 target_pos,
	int line_id)
{
	assert(caster.pos() != target_pos); // Should not be shooting beam with zero trajectory.
	assert(line_id != c_Invalid);

	Creature::Handle intended_target = Creature::creature_at_pos(target_pos);
	int ability_range = Ability::get_range(ability);
	uint flags = f_CasterAimed;

	Beam::Type beam_type = Type::Projectile;
	char const* noun = "ERROR BEAM";
	char const* colour = cstr_White;
	int codepoint = '*';

	if (Ability::target_type(ability) == Ability::TargetType::Melee)
	{
		ability_range = 2;
		beam_type = Type::Melee;
		noun = caster.short_name();
	}
	else if (Ability::target_type(ability) == Ability::TargetType::Projectile)
	{
		Ability::ProjectileData const proj = Ability::get_projectile(ability);
		noun = proj.noun;
		colour = proj.colour;
		codepoint = proj.codepoint;
	}

	return Beam::Data
	{
		.start_pos = caster.pos(),
		.target_pos = target_pos,
		.pos = caster.pos(),
		.type = beam_type,
		.effect_func = Ability::get_effect_func(ability),
		.noun = noun,
		.colour = colour,
		.caster = caster,
		.intended_target = intended_target,
		.codepoint = codepoint,
		.trajectory = line_id,
		.max_range = ability_range,
		.base_accuracy = Ability::get_accuracy(ability),
		.cloud_accuracy_loss = 0,
		.damage_type = Ability::damage_type(ability),
		.damage = Ability::get_damage(ability),
		.spell_power = 0, // not a spell
		.flags = flags,
		.done = false
	};
}

void shoot_beam (Beam::Data & beam)
{
	if (beam.trajectory == LineCache::c_StairsLine)
	{
		shoot_beam_on_stairs(beam);
	}
	else
	{
		shoot_beam_on_line(beam);
	}
}

void shoot_beam_on_line (Beam::Data & beam)
{
	LineCache::Itr3D line_itr(beam.pos, beam.trajectory);

	// init for animation
	Draw::View view = Draw::get_view();

	while (!beam.done)
	{
		// It keeps flying until it hits something.
		line_itr.advance_and_loop();

		// update position
		beam.pos = *line_itr;

		sweep_beam_on_current_pos(beam, view, beam.codepoint, beam.colour, line_itr);
	}
}

void shoot_beam_on_stairs (Beam::Data & beam)
{
	// init for animation
	Draw::View view = Draw::get_view();

	World const& world = World::read();
	Stairs::Direction dir = world.get_stairs(beam.pos);

	assert(dir != Stairs::None);

	Vec3 const move = Stairs::relative_move(dir);
	beam.pos += move;

	// Make a fake line iterator for the horizontal impact (needed for Flipendo).
	Vec2 const move2d = move.xy();
	int const fake_line_id = LineCache::get_lines(move2d).at(0);
	LineCache::Itr3D fake_line(beam.pos, fake_line_id);

	sweep_beam_on_current_pos(beam, view, beam.codepoint, beam.colour, fake_line);

	// If it didn't hit anything, hit the ceiling/floor.
	if (!beam.done)
	{
		if (move.z > 0)
		{
			Draw::pos_message(beam.pos, std::format("The {} hits the ceiling.",
				beam.noun));
		}
		else
		{
			Draw::pos_message(beam.pos, std::format("The {} hits the floor.",
				beam.noun));
		}
		beam.done = true;
	}
}

void sweep_beam_on_current_pos (Beam::Data & beam, Draw::View& view, int codepoint, char const* colour, LineCache::Itr3D const& line_itr)
{
	bool const out_of_range = !within_range(beam.start_pos, beam.pos, beam.max_range);
	if (out_of_range)
	{
		beam.done = true;
	}
	else
	{
		// do animation
		World const& world = World::read();
		if (world.is_visible(beam.pos))
		{
			Draw::draw_tile_temp(codepoint, beam.pos.xy(), view, colour);
		}

		// see if we hit anything; this may also change done to true
		test_for_impact(beam, line_itr);

		if (!beam.done && beam.pos == beam.target_pos
			&& Util::IsFlagSet(beam.flags, f_StopOnTarget))
		{
			detonate_in_midair(beam, line_itr);
			beam.done = true;
		}

		if (!beam.done)
		{
			// Lose accuracy for clouds
			Cloud::Type cloud = World::read().get_cloud(beam.pos);
			beam.cloud_accuracy_loss += Cloud::accuracy_loss(cloud);
		}
	}
}

void test_for_impact (Beam::Data & beam, LineCache::Itr3D const & line)
{
	World const& world = World::read();

	// hit wall
	Terrain::Type t = world.get_terrain(beam.pos);
	if (Terrain::is_solid(t))
	{
		Draw::pos_message(beam.pos, std::format("The {} hits the {}.",
			beam.noun, Terrain::get_name(t)));
		beam.done = true;

		if (Util::IsFlagSet(beam.flags, f_StopOnTarget))
		{
			detonate_in_midair(beam, line);
		}

		return;
	}

	// might hit creature
	Creature::Handle creature_in_path = Creature::creature_at_pos(beam.pos);
	if (creature_in_path != Creature::None)
	{
		// Notify the creature that it's being shot at.
		if (!creature_in_path.is_player())
		{
			Bot::notify_investigate(creature_in_path, beam.start_pos);
		}

		// check accuracy
		int hit_chance = get_hit_chance(beam, creature_in_path);
		int accuracy_roll = Random::in_range(0,99);

		if (Debug::enabled(Debug::Spell))
		{
			std::cout << std::format(" Final Accuracy: {}; roll: {}\n",
				hit_chance, accuracy_roll);
		}

		if (accuracy_roll < hit_chance)
		{
			hit_creature(beam, creature_in_path, line);
			beam.done = true;
			return;
		}
		else
		{
			Draw::creature_message(creature_in_path, std::format("The {} misses {}.",
				beam.noun, Grammar::you(creature_in_path)));
			if (beam.type == Type::Melee)
			{
				beam.done = true;
			}
		}
	}
}

static int get_hit_chance(Beam::Data const & beam, Creature::Handle target)
{
	int caster_accuracy_factor;
	int target_evasion_divisor;
	
	// Range falloff
	int const range_accuracy = accuracy_at_range(beam.base_accuracy, beam.start_pos, beam.pos);

	if (beam.caster == Creature::None || !Util::IsFlagSet(beam.flags, f_CasterAimed))
	{
		// if not aimed by caster, don't factor in caster's accuracy.
		caster_accuracy_factor = 100;
	}
	else
	{
		int const caster_accuracy = beam.caster.accuracy() - beam.cloud_accuracy_loss;

		if (caster_accuracy < -90)
		{
			// minimum caster accuracy is 10%, even if status effects are heavily stacked
			caster_accuracy_factor = 10;
		}
		else if (caster_accuracy > 100 && target != beam.intended_target)
		{
			// don't apply accuracy bonus if the target is not the intended target
			caster_accuracy_factor = 100;
		}
		else
		{
			caster_accuracy_factor = 100 + caster_accuracy;
		}
	}

	// factor in target evasion, capped to prevent divide by zero (or near-certain hit)
	if (target.evasion() < -80)
	{
		target_evasion_divisor = 20;
	}
	else
	{
		target_evasion_divisor = 100 + target.evasion();
	}

	assert(range_accuracy > 0);
	assert(caster_accuracy_factor > 0);
	assert(target_evasion_divisor > 0);

	if (Debug::enabled(Debug::Spell))
	{
		std::cout << std::format(" Base Accuracy {}, Ranged Accuracy {}, Caster Factor {}",
			beam.base_accuracy, range_accuracy, caster_accuracy_factor);
		if (beam.cloud_accuracy_loss > 0)
		{
			std::cout << std::format(" (with -{} from clouds)", beam.cloud_accuracy_loss);
		}
		std::cout << std::format(", Target Divisor {}\n", target_evasion_divisor);
	}

	int const hit_chance = (range_accuracy * caster_accuracy_factor) / target_evasion_divisor;

	if (hit_chance < 1)
	{
		return 1; // never a guaranteed miss
	}
	else if (hit_chance > 99)
	{
		return 99; // never a guaranteed hit
	}
	else
	{
		return hit_chance;
	}
}

void hit_creature(Beam::Data const & beam, Creature::Handle target, LineCache::Itr3D const & line)
{
	// todo - exception for firing into watertrap

	// todo - exception for disintegration field--annihilate non-spell projectiles

	// todo - exception for protego: block or reflect spell

	// Apply effect, then deal damage.
	// We do it in this order so you'll see the hit message before the resist message.
	if (beam.effect_func != nullptr)
	{
		Spell::EffectParams params
		{
			.caster = beam.caster,
			.target = target,
			.target_pos = target.pos(),
			.impact_line = &line
		};

		beam.effect_func(params);
	}
	if (beam.damage > 0)
	{
		target.take_damage(beam.damage, beam.damage_type, beam.caster);
	}
}

void detonate_in_midair (Beam::Data const & beam, LineCache::Itr3D const & line)
{
	if (beam.effect_func != nullptr)
	{
		Spell::EffectParams params
		{
			beam.caster,
			Creature::None,
			beam.pos,
			&line
		};

		beam.effect_func(params);
	}
}

} // namespace Beam
