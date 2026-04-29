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
	//-------------------------------------------------------------------------------------------------
	// Data

	struct Data
	{
		char const* name;
		int codepoint;
		char const* colour;
		int accuray_loss;
		int vision_loss;
		Damage::Type damage_type;
	};

	Cloud::Data const s_data[] = 
	{
		//	 Name		Codepoint						Colour					-Accuracy	-Vision	Damage Type
		Data{"smoke",	Codepoint::BackwardsSquiggle,	cstr_Grey,				30,			2,		Damage::None	},
		Data{"slime",	Codepoint::MidTilde,			cstr_LightChartreuse,	0,			0,		Damage::Acid	},
		Data{"puddle",	Codepoint::MidTilde,			cstr_Sky,				0,			0,		Damage::None	},
	};

	//-------------------------------------------------------------------------
	// Helper function declarations

	void slime_burn(Creature::Handle creature);

	//-------------------------------------------------------------------------
	// Interface

	int get_codepoint(Cloud::Type c)
	{
		assert(is_cloud(c));
		return s_data[c].codepoint;
	}

	char const * get_colour(Cloud::Type c)
	{
		assert(is_cloud(c));
		return s_data[c].colour;
	}

	char const * look_describe(Cloud::Type c)
	{
		switch (c)
		{
			case Cloud::Smoke:		return "- a cloud of smoke";
			case Cloud::Puddle:		return "- a puddle of water";
			case Cloud::Slime:		return "- a puddle of slime";
			default: DebugBreak();	return "- error: unknown cloud type";
		}
	}

	int accuracy_loss (Cloud::Type c)
	{
		assert(is_cloud(c));
		return s_data[c].accuray_loss;
	}

	int vision_loss (Cloud::Type c)
	{
		assert(is_cloud(c));
		return s_data[c].vision_loss;
	}

	bool hazardous_for (Cloud::Type cloud, Creature::Handle const& creature)
	{
		assert(is_cloud(cloud));
		if (s_data[cloud].damage_type == Damage::None)
		{
			return false;
		}
		return !creature.is_immune(s_data[cloud].damage_type);
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
			Damage::Packet const dmg
			{
				.amount = 1,
				.type = Damage::Acid,
				.cause = Damage::Cause(Cloud::Slime)
			};
			creature.take_damage(dmg);
		}
	}
}