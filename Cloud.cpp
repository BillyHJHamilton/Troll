#include "Cloud.h"
#include "Codepoint.h"
#include "Colour.h"
#include "Creature.h"
#include "Damage.h"
#include "Debug.h"
#include "Draw.h"
#include "Grammar.h"
#include <format>

namespace Cloud
{
	//-------------------------------------------------------------------------
	// Helper function declarations

	void slime_burn(Creature::Handle creature);

	//-------------------------------------------------------------------------
	// Interface

	int get_codepoint(Cloud::Type c)
	{
		switch (c)
		{
			case Cloud::Smoke:
				return Codepoint::BackwardsSquiggle;
			case Cloud::Slime:
				return Codepoint::MidTilde;
			default: DebugBreak(); return '?';
		}
	}

	char const * get_colour(Cloud::Type c)
	{
		switch (c)
		{
			case Cloud::Smoke:		return cstr_Grey;
			case Cloud::Slime:		return cstr_LightChartreuse;
			default: DebugBreak();	return cstr_White;
		}
	}

	char const * look_describe(Cloud::Type c)
	{
		switch (c)
		{
			case Cloud::Smoke:		return "- a cloud of smoke";
			case Cloud::Slime:		return "- a puddle of slime";
			default: DebugBreak();	return "- error: unknown cloud type";
		}
	}

	int accuracy_loss (Cloud::Type c)
	{
		switch (c)
		{
			case Cloud::None:		return 0;
			case Cloud::Smoke:		return 30;
			case Cloud::Slime:		return 0;
			default: DebugBreak();	return 0;
		}
	}

	int vision_loss (Cloud::Type c)
	{
		switch (c)
		{
			case Cloud::None:		return 0;
			case Cloud::Smoke:		return 2;
			case Cloud::Slime:		return 0;
			default: DebugBreak();	return 0;
		}
	}

	bool affects_creatures (Cloud::Type cloud)
	{
		switch (cloud)
		{
			case Cloud::Slime:
				return true;
			default:
				return false;
		}
	}

	bool hazardous_for (Cloud::Type cloud, Creature::Handle const& creature)
	{
		switch (cloud)
		{
			case Cloud::Slime:
				return !creature.has_tag(Creature::Tag::Trail_Slime)
					&& !creature.is_immune(Damage::Acid);

			default:
				return false;
		}
	}

	void affect_creature (Cloud::Type cloud, Creature::Handle& creature)
	{
		switch (cloud)
		{
			case Cloud::Slime:
				slime_burn(creature);
				break;

			default:
				// Other clouds have no ill effect.
				break;
		}
	}

	//-------------------------------------------------------------------------
	// Helper function implementations

	void slime_burn(Creature::Handle creature)
	{
		if (hazardous_for(Cloud::Slime, creature))
		{
			Draw::creature_message(creature, std::format("{} burned by the slime.",
				Grammar::You_are(creature)));
			creature.take_damage(1, Damage::Acid, Creature::None);
		}
	}
}