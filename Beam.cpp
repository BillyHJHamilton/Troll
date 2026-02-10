#include "Beam.h"
#include "Cloud.h"
#include "Creature.h"
#include "Debug.h"
#include "Draw.h"
#include "Grammar.h"
#include "Random.h"
#include "Spell.h"
#include "SpellEffect.h"
#include "Stairs.h"
#include "World.h"

#include <cassert>

namespace Beam
{

int constexpr c_accuracy_loss = 3; // per square
int constexpr c_min_ranged_accuracy = 40; // before character stats are applied

// ------------------------------------------------------------------------------------------------
// helper function declarations

static Beam::Data make_spell_beam (Spell::Index, Creature::Handle caster, Vec3 target_pos,
	bool caster_aimed, int line_id);
static void shoot_beam (Beam::Data & beam);
static void shoot_beam_on_line (Beam::Data & beam);
static void shoot_beam_on_stairs (Beam::Data & beam);
static void sweep_beam_on_current_pos (Beam::Data & beam, Draw::View& view, int codepoint,
	std::string& colour, LineCache::Itr3D const& line_itr);
static void test_for_impact (Beam::Data & beam, LineCache::Itr3D const & line_itr);
static std::string beam_description (Beam::Data const & beam);
static int get_hit_chance (Beam::Data const & beam, Creature::Handle target);
static void hit_creature (Beam::Data const & beam, Creature::Handle target,
	LineCache::Itr3D const & line);
static void detonate_in_midair (Beam::Data const & beam, LineCache::Itr3D const & line);

static std::string get_colour (Beam::Data const & beam);
static int get_codepoint (Beam::Data const & beam);
static int get_damage (Beam::Data const & beam);
static Spell::EffectFunc get_effect_func (Beam::Data const & beam);

// ------------------------------------------------------------------------------------------------
// interface function implementations

void shoot_spell (Spell::Index spell, Creature::Handle caster, Vec3 target_pos,
	bool caster_aimed, int line_id)
{
	Spell::create_and_bind_instance(spell, caster);
	Beam::Data beam = make_spell_beam(spell, caster, target_pos, caster_aimed, line_id);
	shoot_beam(beam);
}

int accuracy_at_range(int base_accuracy, Vec3 start, Vec3 end)
{
	int const dist = (int)euclidean_distance(start, end);
	int const loss = dist * c_accuracy_loss;
	return std::max(c_min_ranged_accuracy, base_accuracy - loss);
}

// ------------------------------------------------------------------------------------------------
// helper function implementations

Beam::Data make_spell_beam (Spell::Index spell, Creature::Handle caster, Vec3 target_pos,
	bool caster_aimed, int line_id)
{
	assert(caster.pos() != target_pos); // Should not be shooting beam with zero trajectory.
	assert(line_id != c_invalid);

	Creature::Handle intended_target = Creature::None;
	if (caster_aimed)
	{
		intended_target = Creature::creature_at_pos(target_pos);
	}

	int const spell_range = Spell::get_range(spell);
	const World& world = World::read();
	const bool stop_on_target = (Spell::get_target_type(spell) == Spell::TargetType::Tile);

	return Beam::Data
	{
		caster.pos(),
		target_pos,
		caster.pos(),
		Beam::Type::Spell,
		caster,
		intended_target,
		line_id,
		spell_range,
		0 /*cloud accuracy loss*/,
		caster_aimed,
		false,
		stop_on_target
	};
}

void shoot_beam (Beam::Data & beam)
{
	if (beam.trajectory == LineCache::c_stairs_line)
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
	int codepoint = get_codepoint(beam);
	std::string colour = get_colour(beam);

	while (!beam.done)
	{
		// It keeps flying until it hits something.
		line_itr.advance_and_loop();

		// update position
		beam.pos = *line_itr;

		sweep_beam_on_current_pos(beam, view, codepoint, colour, line_itr);
	}
}

void shoot_beam_on_stairs (Beam::Data & beam)
{
	// init for animation
	Draw::View view = Draw::get_view();
	int codepoint = get_codepoint(beam);
	std::string colour = get_colour(beam);

	World const& world = World::read();
	Stairs::Direction dir = world.get_stairs(beam.pos);

	assert(dir != Stairs::None);

	Vec3 const move = Stairs::relative_move(dir);
	beam.pos += move;

	// Make a fake line iterator for the horizontal impact (needed for Flipendo).
	Vec2 const move2d = move.xy();
	int const fake_line_id = LineCache::get_lines(move2d).at(0);
	LineCache::Itr3D fake_line(beam.pos, fake_line_id);

	sweep_beam_on_current_pos(beam, view, codepoint, colour, fake_line);

	// If it didn't hit anything, hit the ceiling/floor.
	if (!beam.done)
	{
		if (move.z > 0)
		{
			Draw::pos_message(beam.pos, " The " + beam_description(beam) + " hits the ceiling.");
		}
		else
		{
			Draw::pos_message(beam.pos, " The " + beam_description(beam) + " hits the floor.");
		}
		beam.done = true;
	}
}

void sweep_beam_on_current_pos (Beam::Data & beam, Draw::View& view, int codepoint, std::string& colour, LineCache::Itr3D const& line_itr)
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
			Draw::draw_tile_temp(codepoint, beam.pos.xy(), view, colour.c_str());
		}

		// see if we hit anything; this may change done to true
		test_for_impact(beam, line_itr);

		if (!beam.done && beam.pos == beam.target_pos && beam.stop_on_target)
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
	if (world.is_solid(beam.pos))
	{
		Draw::pos_message(beam.pos, " The " + beam_description(beam) + " hits the wall.");
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

		if (c_ShowSpellDebug)
		{
			std::cout << " Final Accuracy: " << hit_chance
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
			Draw::add_message(" The " + beam_description(beam) + " misses "
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
	int caster_accuracy_factor;
	int target_evasion_divisor;

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

	if (c_ShowSpellDebug)
	{
		std::cout << " Base Accuracy: " << base_accuracy
			<< ", Ranged Accuracy: " << range_accuracy
			<< ", Caster Factor: " << caster_accuracy_factor;
		if (beam.cloud_accuracy_loss > 0)
		{
			std::cout << " (with -" << beam.cloud_accuracy_loss << " from clouds)";
		}
		std::cout << ", Target Divisor: " << target_evasion_divisor << "\n";
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

	int damage = get_damage(beam);
	Spell::EffectFunc effect_func = get_effect_func(beam);

	// deal damage and then apply effect
	target.take_damage(damage, beam.caster);
	if (effect_func != nullptr)
	{
		Spell::EffectParams params
		{
			beam.caster,
			target,
			target.pos(),
			&line
		};

		effect_func(params);
	}
}

void detonate_in_midair (Beam::Data const & beam, LineCache::Itr3D const & line)
{
	Spell::EffectFunc effect_func = get_effect_func(beam);
	if (effect_func != nullptr)
	{
		Spell::EffectParams params
		{
			beam.caster,
			Creature::None,
			beam.pos,
			&line
		};

		effect_func(params);
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

