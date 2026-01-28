#include "Beam.h"
#include "Creature.h"
#include "Draw.h"
#include "Grammar.h"
#include "Random.h"
#include "Spell.h"
#include "World.h"

#include <cassert>

namespace Beam
{

// Line already advanced to point of impact.  Used for Flipendo pushback.
static std::optional<LineCache::Itr3D> s_impact_line;

int constexpr c_accuracy_loss = 3; // per square
int constexpr c_min_ranged_accuracy = 40; // before character stats are applied

// ------------------------------------------------------------------------------------------------
// helper function declarations

static Beam::Data make_spell_beam (Spell::Index, Creature::Handle caster, Vec3 target_pos, bool caster_aimed);
static void shoot_beam (Beam::Data & beam);
static void test_for_impact (Beam::Data & beam, LineCache::Itr3D const & line);
static std::string beam_description (Beam::Data const & beam);
static int get_hit_chance (Beam::Data const & beam, Creature::Handle target);
static void hit_creature (Beam::Data const & beam, Creature::Handle target, LineCache::Itr3D const & line);

static std::string get_colour (Beam::Data const & beam);
static int get_codepoint (Beam::Data const & beam);
static int get_damage (Beam::Data const & beam);
static Spell::EffectFunc get_effect_func (Beam::Data const & beam);

// ------------------------------------------------------------------------------------------------
// interface function implementations

void shoot_spell (Spell::Index spell, Creature::Handle caster, Vec3 target_pos, bool caster_aimed)
{
	Spell::create_and_bind_instance(spell, caster);
	Beam::Data beam = make_spell_beam(spell, caster, target_pos, caster_aimed);
	shoot_beam(beam);
}

int accuracy_at_range(int base_accuracy, Vec3 start, Vec3 end)
{
	int const dist = (int)euclidean_distance(start, end);
	int const loss = dist * c_accuracy_loss;
	return std::max(c_min_ranged_accuracy, base_accuracy - loss);
}

std::optional<LineCache::Itr3D> get_latest_impact_line ()
{
	return s_impact_line;
}

// ------------------------------------------------------------------------------------------------
// helper function implementations

Beam::Data make_spell_beam (Spell::Index spell, Creature::Handle caster, Vec3 target_pos, bool caster_aimed)
{
	assert(caster.pos() != target_pos); // Should not be shooting beam with zero trajectory.

	Creature::Handle intended_target = Creature::None;
	if (caster_aimed)
	{
		intended_target = Creature::creature_at_pos(target_pos);
	}

	// Find line trajectory.
	// TODO: Properly handle case where target is not in the line cache.
	// Probably we should get the line earlier and abort on failure.
	// For now it's just being set to 0 below.

	const World& world = World::read();
	const int trajectory = world.get_los(caster.pos(), target_pos);

	return Beam::Data
	{
		caster.pos(),
		caster.pos(),
		Beam::Type::Spell,
		caster,
		intended_target,
		(trajectory == c_invalid) ? 0 : trajectory,
		Spell::get_range(spell),
		caster_aimed,
		false
	};
}

void shoot_beam (Beam::Data & beam)
{
	LineCache::Itr3D line_itr(beam.pos, beam.trajectory);

	// init for animation
	Draw::View view = Draw::get_view();
	int codepoint = get_codepoint(beam);
	std::string colour = get_colour(beam);

	while (!beam.done)
	{
		// It keeps flying until it hits something.
		line_itr.advance_and_loop();

		// update position
		beam.pos = *line_itr;

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
				Draw::draw_tile_temp(codepoint, beam.pos.xy(), view, colour.c_str());
			}

			// see if we hit anything; this may change done to true
			test_for_impact(beam, line_itr);
		}
	}
}

void test_for_impact (Beam::Data & beam, LineCache::Itr3D const & line)
{
	World const& world = World::read();

	// hit wall
	if (world.is_solid(beam.pos))
	{
		if (world.get_visibility(beam.pos) == Visibility::Visible)
		{
			Draw::add_message("The " + beam_description(beam) + " hits the wall.");
		}
		beam.done = true;
		return;
	}

	// might hit creature
	int creature_in_path = Creature::creature_at_pos(beam.pos);
	if (creature_in_path != Creature::None)
	{
		// check accuracy
		int hit_chance = get_hit_chance(beam, creature_in_path);
		int accuracy_roll = Random::in_range(0,99);

		if (SHOW_SPELL_DEBUG)
		{
			std::cout << "Hit chance: " << hit_chance
				<< "; roll: " << accuracy_roll << std::endl;
		}

		if (accuracy_roll < hit_chance)
		{
			hit_creature(beam, creature_in_path, line);
			beam.done = true;
			return;
		}
		else
		{
			Draw::add_message("The " + beam_description(beam) + " misses "
				+ Grammar::you(creature_in_path) + ".");
		}
	}
}

static std::string beam_description(Beam::Data const & beam)
{
	if (beam.type == Beam::Type::Spell)
	{
		return "spell";
	}
	else
	{
		return "flying motorcycle"; // todo!
	}
}

static int get_hit_chance(Beam::Data const & beam, Creature::Handle target)
{
	int base_accuracy;
	Creature::Handle caster_accuracy_factor;
	Creature::Handle target_evasion_divisor;

	if (beam.type == Beam::Type::Spell)
	{
		// if it's a spell, the data is in the bound spell instance
		Spell::Instance const & spell_instance = Spell::get_current_instance();
		base_accuracy = spell_instance.accuracy;
	}
	else
	{
		// todo - projectiles
		base_accuracy = 50;
	}

	// Range falloff
	int const range_accuracy = accuracy_at_range(base_accuracy, beam.start_pos, beam.pos);

	if (beam.caster == Creature::None || !beam.caster_aimed)
	{
		// if not aimed by caster, don't factor in caster's accuracy.
		caster_accuracy_factor = 100;
	}
	else if (beam.caster.accuracy() < -90)
	{
		// minimum caster accuracy is 10%, even if status effects are heavily stacked
		caster_accuracy_factor = 10;
	}
	else if (beam.caster.accuracy() > 100 && target != beam.intended_target)
	{
		// don't apply accuracy bonus if the target is not the intended target
		caster_accuracy_factor = 100;
	}
	else
	{
		caster_accuracy_factor = 100 + beam.caster.accuracy();
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

	int damage = get_damage(beam);
	Spell::EffectFunc effect_func = get_effect_func(beam);

	// stash a copy of the impact line (used by Flipendo pushback)
	s_impact_line = line;

	// deal damage and then apply effect
	target.take_damage(damage, beam.caster);
	if (effect_func != nullptr)
	{
		effect_func(beam.caster, target);
	}
}

std::string get_colour (Beam::Data const & beam)
{
	if (beam.type == Beam::Type::Spell)
	{
		Spell::Instance inst = Spell::get_current_instance();
		return inst.colour;
	}
	else
	{
		return "white"; // todo - projectiles
	}
}

int get_codepoint (Beam::Data const & beam)
{
	if (beam.type == Beam::Type::Spell)
	{
		Spell::Instance inst = Spell::get_current_instance();
		return inst.codepoint;
	}
	else
	{
		return '*'; // todo - projectiles
	}
}

int get_damage (Beam::Data const & beam)
{
	if (beam.type == Beam::Type::Spell)
	{
		Spell::Instance inst = Spell::get_current_instance();
		return inst.damage;
	}
	else
	{
		return 1; // todo - projectiles
	}
}

static Spell::EffectFunc get_effect_func (Beam::Data const & beam)
{
	if (beam.type == Beam::Type::Spell)
	{
		Spell::Instance inst = Spell::get_current_instance();
		return inst.effect_func;
	}
	else
	{
		return nullptr; // todo - projectiles
	}
}

} // namespace Beam

