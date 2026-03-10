#pragma once

#include "Types.h"
#include "VectorUtil.h"

namespace Taunt
{
	// Is the taunt quoted speech, or presented as an action?
	enum Presentation : byte
	{
		Say,
		Emote
	};

	// Controls when the taunt can appear.
	enum Condition : byte
	{
		AnyTime,
		Greeting,
		AttackSpell,	// Subtype is spell NPC is casting, or -1 for any.
		PlayerMiscast,	// Subtype is spell player miscasted, or -1 for any.
		HasStatus,		// Subtype is status NPC is enjoying.  Don't use -1.
		PlayerStatus,	// Subtype is status the player is enjoying.  Don't use -1.
		Losing,
		Winning,
		FollowUp,		// Used immediately after previous taunt.
	};

	struct Data
	{
		// Permanent data
		char const* text = nullptr;
		Presentation presentation = Say;
		Condition condition = AnyTime;
		int subtype = c_Invalid; // more details for condition
		int rarity = 1; // 1/rarity chance of considering this taunt
		bool format = false; // Does the taunt need std::vformat?
		bool repeat = false; // Can it appear more than once per game?

		// Runtime data
		int uses = 0;

		void serialize(ISerializer& s);
	};

	void init();
	void clear();
	void serialize(ISerializer& s);

	void find_taunts(Creature::Handle taunter, Condition condition, int subtype,
		IntTempList& out_taunts);
	void find_status_taunts(Creature::Handle taunter, Creature::Handle target,
		IntTempList& out_taunts);
	int find_followup(Creature::Handle taunter, int last_taunt);
	void say_taunt(Creature::Handle taunter, int taunt_id);
}
