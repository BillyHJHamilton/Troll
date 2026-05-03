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

	bool is_slime_hazardous_for(Creature::Handle const& creature);
	void slime_burn(Creature::Handle& creature);

	//-------------------------------------------------------------------------------------------------
	// Data

	using IsHazardousFunc = bool(*)(Creature::Handle const& creature);
	using EffectFunc = void(*)(Creature::Handle& creature);

	struct Data
	{
		char const* name;
		int codepoint;
		char const* colour;
		int accuray_loss;
		int vision_loss;
		IsHazardousFunc is_hazardous_func;
		EffectFunc effect_func;
	};

	Cloud::Data const s_data[] = 
	{
		//	 Name		Codepoint						Colour					-Accuracy	-Vision	Is Hazardous Function?	Effect Function
		Data{"smoke",	Codepoint::BackwardsSquiggle,	cstr_Grey,				30,			2,		nullptr,				nullptr	},
		Data{"slime",	Codepoint::MidTilde,			cstr_LightChartreuse,	0,			0,		is_slime_hazardous_for,	slime_burn	},
	};

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

	bool affects_creatures(Cloud::Type cloud)
	{
		if (!is_cloud(cloud))
		{
			return false;
		}
		return s_data[cloud].is_hazardous_func != nullptr;
	}
	
	bool hazardous_for(Cloud::Type cloud, Creature::Handle const& creature)
	{
		if (!is_cloud(cloud))
		{
			return false;
		}
		if (s_data[cloud].is_hazardous_func == nullptr)
		{
			return false;
		}
		return s_data[cloud].is_hazardous_func(creature);
	}

	void affect_creature (Cloud::Type cloud, Creature::Handle& creature)
	{
		if (is_cloud(cloud))
		{
			if (s_data[cloud].effect_func != nullptr)
			{
				s_data[cloud].effect_func(creature);
			}
		}
	}

	//-------------------------------------------------------------------------
	// Helper function implementations

	bool is_slime_hazardous_for(Creature::Handle const& creature)
	{
		return !creature.is_immune(Damage::Acid);
	}

	void slime_burn(Creature::Handle& creature)
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