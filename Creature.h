#pragma once

#include "Types.h"
#include "Geometry.h"

#include <iostream>

// Creatures are people and other living beings in the game.
// Most creatures are the "bad guys", but there is also a creature for the player.
// This keeps the code simple as we can use the same functions for player and NPC.
// Concerns that apply *only* to the player, such as XP, will be handled elsewhere.

// Individual creatures are identified by index (a simple int) throughout the program.
// The index of Creature::Player == 0 is reserved for the player.
// Don't store the indices to check a creature's identity, since a removed creature's
// index may be reused.  If necessary we can add a separate ID for each individual.

// Each creature has a type (Creature::Type) enum.  Don't confuse index with type.
// There may be multiple creatures of the same type, though hopefully not for named
// characters like Neville or Dumbledore.

struct DrawView;

enum class Gender
{
	Male,
	Female,
	Neuter
};

namespace Creature
{
	enum Type : int
	{
		None = -1,	 // not included in count
		Player = 0,
		Neville_0,
		ColinCreevy_0,
		Count
	};

	struct Stats
	{
		Type type;
		char const * name;
		int codepoint;
		Gender gender;
		int skill_magic;
		int max_hp;

		int hp;
		Vec2 pos;
	};

	struct DerivedStats
	{
		int distractedness;
		int miscastiness;
		int evasion;
		int accuracy;
		int shield_strength;
	};
};

extern std::vector<int> g_visible_creatures;

void init_creatures ();
void clear_creatures ();

bool creature_valid (int creature_index);
Creature::Type creature_type (int creature_index);
std::string creature_name (int creature_index);
Gender creature_gender (int creature_index);
int creature_skill_magic (int creature_index);
int creature_max_hp (int creature_index);
int creature_hp (int creature_index);
Vec2 const & creature_pos (int creature_index);
bool creature_has_status (int creature_index, Status::Index status);
int creature_status_severity (int creature_index, Status::Index status);
int creature_distractedness (int creature_index);
int creature_miscastiness (int creature_index);
int creature_evasion (int creature_index);
int creature_accuracy (int creature_index);

int creature_at_pos (Vec2 pos);
bool creature_is_player (int creature_index);
bool creature_visible (int creature_index);
std::string creature_status_string (int creature_index);

int spawn_creature (Creature::Type type, Vec2 const & pos);
void damage_creature (int creature_index, int damage);
void move_creature (int creature_index, Vec2 const & new_pos);
void inflict_status (int creature_index, Status::Index status, int severity);
void reduce_status (int creature_index, Status::Index status, int reduction);
void cure_status (int creature_index, Status::Index status);
void creature_cure_all (int creature_index); // heals status and hp

void update_derived_stats (int creature_index);

void update_visible_creatures ();
void draw_creature (int creature_index, DrawView const & view);
void draw_visible_creatures (DrawView const & view);


