#include "Action.h"

#include "Ability.h"
#include "Beam.h"
#include "Creature.h"
#include "Debug.h"
#include "Draw.h"
#include "Grammar.h"
#include "Item.h"
#include "Inventory.h"
#include "Map.h"
#include "Player.h"
#include "Random.h"
#include "Spell.h"
#include "SpellEffect.h"
#include "Target.h"
#include "Terrain.h"
#include "World.h"

#include <cassert>
#include <format>

namespace Action
{

//-------------------------------------------------------------------------------------------------
// Helper function declarations

bool check_distraction (Creature::Handle caster);
bool check_miscast (Creature::Handle caster, Spell::Index spell);
void do_miscast (Creature::Handle caster, Spell::Index spell, Vec3 target_pos, int line_id);
void do_successful_cast (Creature::Handle caster, Spell::Index spell, Vec3 target_pos, int line_id);

//-------------------------------------------------------------------------------------------------
// Interface functions

void player_look_at()
{
	if (!Target::is_valid())
	{
		Draw::add_message("Target a tile, then press 'x' to look.");
		return;
	}

	Vec3 const pos = Target::get_pos().value();

	if (!World::read().is_visible(pos))
	{
		Draw::add_message("You can't see that tile.");
		return;
	}

	Draw::add_message("You see:");
	bool printed = false;

	if (Player::pos() != pos)
	{
		Creature::Handle creature = Creature::creature_at_pos(pos);
		if (creature.valid())
		{
			Draw::add_message(std::format("- {}", creature.long_name()));
			printed = true;
		}
	}

	Cloud::Type cloud = World::read().get_cloud(pos);
	if (cloud != Cloud::None)
	{
		Draw::add_message(Cloud::look_describe(cloud));
		printed = true;
	}
	else // cloud blocks terrain and items
	{
		Item::Handle item = World::read().peek_item(pos);
		while (item.valid())
		{
			Draw::add_message(std::format("- {}", item.name()));
			item = item.next_in_stack();
				printed = true;
		}

		Terrain::Type t = World::read().get_terrain(pos);
		if (t != Terrain::Open)
		{
			// stairs trick when looking at a different map
			if (pos.z != Player::pos().z)
			{
				t = Terrain::swap_stairs(t);
			}

			Draw::add_message(Terrain::look_describe(t));
			printed = true;
		}
	}

	if (!printed)
	{
		Draw::add_message("- nothing of interest");
	}
}

void player_rest_step()
{
	Player::handle().rest_step();
	Player::set_acted(true);
}

bool player_try_move(Vec2 relative_move)
{
	bool const moved = try_move(Player::handle(), relative_move, MoveMode::Walk);
	if (moved)
	{
		// Pick up items.
		Item::Handle item = World::edit().pop_item(Player::pos());
		while (item != c_Invalid)
		{
			Inventory::edit().add_item(item);
			Draw::add_message("Got " + item.name() + ".");
			item = World::edit().pop_item(Player::pos());
		}

		Player::set_acted(true);
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

	std::optional<Vec3> target_pos = Target::get_pos();
	Spell::TargetType target_type = Spell::get_target_type(spell);

	if (target_type == Spell::TargetType::Self)
	{
		try_cast_spell(spell, Creature::Player, Player::pos(), c_Invalid);
		Player::set_acted(true);
		return true;
	}
	else
	{
		// It's a targeted spell.  Check that targeting is valid.

		if (!target_pos.has_value())
		{
			Draw::add_message("No target.");
			return false;
		}

		if (target_pos == Player::pos())
		{
			// Special case for shooting yourself.
			//Draw::add_message("Don't shoot that spell at yourself.");
			try_cast_spell(spell, Creature::Player, Player::pos(), c_Invalid);
			Player::set_acted(true);
			return true;
		}
		
		if (!within_range(Player::pos(), target_pos.value(), Spell::get_range(spell)))
		{
			Draw::add_message("The target is out of range.");
			return false;
		}

		int line_id = World::read().get_los(Player::pos(), target_pos.value(),
			Player::vision_radius);
		if (line_id == c_Invalid)
		{
			Draw::add_message("Target not visible.");
			return false;
		}

		if (target_type == Spell::TargetType::Sight)
		{
			// Don't actually use the line - it's a sight-targed spell.
			// This will make the spell automatically affect the target square.
			line_id = c_Invalid;
		}

		// Having confirmed it is plausible for the player to try to cast the spell,
		// we now continue to the generic spell-casting function.
		try_cast_spell(spell, Creature::Player, target_pos.value(), line_id);

		Player::set_acted(true);
		return true;
	}
}

void player_use_item(int inventory_slot)
{
	Inventory::edit().use_item(inventory_slot);
	Player::set_acted(true);
}

bool try_move (Creature::Handle creature, Vec2 relative_move, MoveMode move_mode)
{
	World const& world = World::read();
	Vec3 old_pos = creature.pos();

	Vec2 new_pos = old_pos.xy() + relative_move;
	Vec3 new_pos_3d = {new_pos.x, new_pos.y, old_pos.z};
	new_pos_3d.z += world.get_stairs_dz(old_pos, new_pos);

	Creature::Handle creature_in_way = Creature::creature_at_pos(new_pos_3d);
	if (creature_in_way != Creature::None)
	{
		return false;
	}
	else if (world.is_solid(new_pos_3d))
	{
		return false;
	}
	else
	{
		creature.clear_rest_steps();

		if (move_mode == MoveMode::Walk)
		{
			int const failure = creature.walk_failure();
			int const roll = Random::in_range(0, 99);
			if (Debug::enabled(Debug::Action) && failure > 0)
			{
				std::cout << std::format("Walk failure ({0}): {1}; roll: {2}\n",
					creature.short_name(), failure, roll);
			}

			if (roll < failure)
			{
				Draw::creature_message(creature, std::format("{} {} to walk.",
					Grammar::You(creature), Grammar::verbs("fail", creature)));
				return true;
			}
		}

		creature.move(new_pos_3d);

		return true;
	}
}

void try_cast_spell (Spell::Index spell, Creature::Handle caster, Vec3 target_pos, int line_id)
{
	caster.clear_rest_steps();

	// Update the screen because we'll do some animation for the spell
	Draw::draw_screen();

	if (Debug::enabled(Debug::Spell))
	{
		std::cout << caster.short_name() << " casting " << Spell::get_name(spell) << "\n";
	}

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
		do_miscast(caster, spell, target_pos, line_id);
		return;
	}

	// 4. Success!
	do_successful_cast(caster, spell, target_pos, line_id);
}

void try_use_ability (Ability::Index ability, Creature::Handle user, Vec3 target_pos, int line_id)
{
	user.clear_rest_steps();

	// Distractedness check.
	// TODO: Should it be lower for abilities/items than for spells?
	bool is_distracted = check_distraction(user);
	if (is_distracted)
	{
		Draw::add_message(Grammar::You_are(user) + " too distracted to attack!");
		return;
	}

	// Update the screen in case we'll do animation for the ability
	Draw::draw_screen();

	// TODO Describe ability activation?
	// More relevant to some abilities than others.

	Ability::TargetType target_type = Ability::target_type(ability);

	if (target_type == Ability::TargetType::Melee)
	{
		Beam::shoot_ability(ability, user, target_pos, line_id);
		return;
	}

	if (target_type == Ability::TargetType::Self)
	{
		if (Ability::is_damaging(ability))
		{
			int const dmg = Ability::get_damage(ability);
			user.take_damage(dmg, user);
		}

		Spell::EffectParams params
		{
			.caster = user,
			.target = Creature::None,
			.target_pos = user.pos(),
			.impact_line = nullptr
		};
		Ability::execute_effect(ability, params);
	}
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
	if (Debug::enabled(Debug::Spell))
	{
		std::cout << std::format(" Distraction Rate: {}%; Roll: {}\n",
			distraction_rate, distractedness_roll);
	}

	return (distractedness_roll < distraction_rate);
}

bool check_miscast (Creature::Handle caster, Spell::Index spell)
{
	// Chance you mess up the spell, based on its difficulty and your skill.
	// This is done with float math since there's an exponent in the formula.
	float miscast_rate = caster.miscast_rate_for_spell(spell);
	float miscast_roll = Random::in_range(0.0f, 100.0f);

	if (Debug::enabled(Debug::Spell))
	{
		std::cout << std::format(" Miscast Rate: {}%; Roll: {}\n",
			miscast_rate, miscast_roll);
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

void do_miscast (Creature::Handle caster, Spell::Index spell, Vec3 target_pos, int line_id)
{
	if (caster.visible())
	{
		Draw::add_message(Grammar::You(caster)
			+ " " + Grammar::verbs("miscast", caster)
			+ " " + Spell::get_name(spell) + "!");

		Draw::View view = Draw::get_view();
		Draw::draw_tile_temp('X', caster.pos().xy(), view, "yellow");
		Draw::draw_tile_temp('X', caster.pos().xy(), view, "black");
	}

	// todo - proper miscasts
	//Miscast::perform(caster, target, spell_used, line_id ...);
}

void do_successful_cast (Creature::Handle caster, Spell::Index spell, Vec3 target_pos, int line_id)
{
	Draw::creature_message(caster, Grammar::You(caster) + " "
		+ Grammar::verbs("cast", caster) + " "
		+ Spell::get_name(spell) + "!");

	if (Spell::get_target_type(spell) == Spell::TargetType::Self)
	{
		Spell::EffectParams params
		{
			.caster = caster,
			.target = Creature::None,
			.target_pos = caster.pos(),
			.impact_line = nullptr
		};

		Spell::execute_effect(spell, params);
	}
	else if (line_id == c_Invalid)
	{
		// Spell is configured to go off without a beam.

		Creature::Handle target = Creature::creature_at_pos(target_pos);
		if (target.valid())
		{
			int const damage = Spell::get_damage(spell, caster);
			target.take_damage(damage, caster);
		}

		Spell::EffectParams params
		{
			.caster = caster,
			.target = target,
			.target_pos = target_pos,
			.impact_line = nullptr
		};

		Spell::execute_effect(spell, params);
	}
	else
	{
		bool constexpr caster_aimed = true;
		Beam::shoot_spell (spell, caster, target_pos, caster_aimed, line_id);
	}
}

} // namespace Action
