#include "Feature.h"

#include "Damage.h"
#include "Debug.h"
#include "Draw.h"
#include "Item.h"
#include "Pathfind.h"
#include "Random.h"
#include "Serialize.h"
#include "Terrain.h"
#include "VectorUtil.h"
#include "World.h"

namespace Feature
{

//-------------------------------------------------------------------------------------------------
// Data

// Feature Types:
//  - Chest - payload is Item::Handle
//  - Desk - no payload
//  - TorchUnlit - payload is which trigger id it activates
// 	             - trigger only activates when the last torch with that trigger is lit
//  - TorchLit - no payload
//  - Portrait - no payload
//  - FlipendoButton - payload is which trigger id it activates
//                   - it also turns into a wall on that trigger
//  - SlidingWall - payload is which trigger id it responds to
//  - Portcullis - payload is which trigger id it responds to

struct Instance
{
	Vec3 pos;
	int hp;

	// Parameters to be interpreted based on type of feature.
	int payload;
};
std::vector<Feature::Instance> s_features;

int s_next_trigger_id = 0;

enum class Material
{
	Stone,
	Metal,
	Wood,
	Count,
};

float constexpr c_Resistances[(int)(Material::Count)][Damage::Type::Count] =
{
	//	Basic	ToLife	Fire	Acid
	{	0.0f,	0.0f,	0.0f,	0.0f,	},	// Stone
	{	0.0f,	0.0f,	0.0f,	1.0f,	},	// Metal
	{	1.0f,	0.0f,	2.0f,	0.0f,	},	// Wood
};

//-------------------------------------------------------------------------------------------------
// Helper declarations

int find_feature(Vec3 pos);
void add_feature_internal(Vec3 pos, Terrain::Type terrain, int hp, int payload);

void init_chest(Feature::Instance& feature);
void init_desk(Feature::Instance& feature);
void init_scanner(Feature::Instance& feature);

void damage_basic(Vec3 pos, Damage::Packet const& damage_packet,
                  Material const material, std::string const name);

void light_torch(Vec3 pos);
bool is_any_unlit_torch_with_trigger(int trigger);

void trigger_all(int trigger);
void trigger_flipendo_button(Feature::Instance & feature);
void trigger_sliding_wall(Feature::Instance & feature);
void trigger_portcullis(Feature::Instance & feature);
void trigger_shop_seed(Feature::Instance & feature);

//-------------------------------------------------------------------------------------------------
// Module interface

void init()
{
	s_features.reserve(30);
}

void clear()
{
	s_features.clear();
	s_next_trigger_id = 0;
}

void serialize(ISerializer& s)
{
	s.srz_vector(s_features, "Feature::s_chests");
	s.srz_int(s_next_trigger_id);
}

int get_new_trigger()
{
	++s_next_trigger_id;
	return s_next_trigger_id - 1;
}

void spawn(Vec3 pos, Terrain::Type type)
{
	if (Check(Terrain::is_feature(type)))
	{
		add_feature_internal(pos, type, c_Invalid, c_Invalid);

		// Feature-specific initialization.
		switch (type)
		{
			case Terrain::Chest:
				init_chest(s_features.back());
				break;
			case Terrain::Desk:
				init_desk(s_features.back());
				break;
			// special initialization
			case Terrain::Scanner:
			case Terrain::FlipendoButton:
			case Terrain::SlidingWall:
			case Terrain::Portcullis:
			case Terrain::ShopSeed:
				DebugBreak("Spawn Feature with trigger");
				break;
			// no initialization needed
			// case Terrain::Armour:
			// case Terrain::TorchUnlit:  // cosmetic torch, can also spawn as trigger
			// case Terrain::TorchLit:
			// case Terrain::Portrait:
		}
	}
}

void spawn(Vec3 pos, Terrain::Type type, int trigger)
{
	if (Check(Terrain::is_feature(type)))
	{
		add_feature_internal(pos, type, c_Invalid, trigger);

		// Feature-specific initialization.
		switch (type)
		{
			case Terrain::Scanner:
				init_scanner(s_features.back());
				break;
			case Terrain::TorchUnlit:  // can also spawn as cosmetic (no trigger)
			case Terrain::FlipendoButton:
			case Terrain::SlidingWall:
			case Terrain::Portcullis:
			case Terrain::ShopSeed:
				break;
			default:
				DebugBreak("Spawn Feature without trigger");
				break;
		}
	}
}

void move(Vec3 old_pos, Vec3 new_pos)
{
	int const index = find_feature(old_pos);
	if (index != c_Invalid)
	{
		Terrain::Type terrain = World::read().get_terrain(old_pos);
		World::edit().set_terrain(old_pos, Terrain::Open);
		World::edit().set_terrain(new_pos, terrain);

		s_features[index].pos = new_pos;
	}
}

void remove(Vec3 pos, Terrain::Type new_terrain_type)
{
	assert(!Terrain::is_feature(new_terrain_type));

	int const index = find_feature(pos);
	if (index != c_Invalid)
	{
		Util::RemoveSwap(s_features, index);
		World::edit().set_terrain(pos, new_terrain_type);
	}
}

void remove(Vec3 pos)
{
	remove(pos, Terrain::Open);
}

//-------------------------------------------------------------------------------------------------
// Helper function implementations

int find_feature(Vec3 pos)
{
	return Util::FindIndexByKey(s_features, &Feature::Instance::pos, pos);
}

void add_feature_internal(Vec3 pos, Terrain::Type terrain, int hp, int payload)
{
	World::edit().set_terrain(pos, terrain);
	s_features.push_back({
		.pos = pos,
		.hp = hp,
		.payload = payload,
		});
}

void damage(Vec3 pos, Damage::Packet const& damage_packet)
{
	switch (World::read().get_terrain(pos))
	{
	case Terrain::Desk:
		damage_basic(pos, damage_packet, Material::Wood, "desk");
		break;
	case Terrain::TorchUnlit:
		if (damage_packet.type == Damage::Type::Fire)
		{
			light_torch(pos);
		}
		break;
	}
}

//-------------------------------------------------------------------------------------------------
// Feature-specific functions

void init_chest(Feature::Instance& feature)
{
	// Todo: Variable treasure based on current map

	Item::Handle top(c_Invalid);

	if (Random::coinflip())
	{
		Item::Handle new_potion = Item::make_potion_by_level(
			World::read().find_map_difficulty(feature.pos) + 1.0f);
		new_potion.stack_onto(top);
		top = new_potion;
	}

	int const num_beans = Random::in_range(3,6);
	for (int i = 0; i < num_beans; ++i)
	{
		Item::Handle new_bean = Item::make_bbb();
		new_bean.stack_onto(top);
		top = new_bean;
	}

	feature.payload = (int)top;
}

void open_chest(Vec3 pos)
{
	int const feature_index = find_feature(pos);
	if (Check(feature_index != c_Invalid))
	{
		Draw::pos_message(pos, "The chest bursts open!");

		Vec3TempList open_pos;
		open_pos.reserve(9);
		Pathfind::find_open_neighbours(pos, {}, open_pos);
		open_pos.push_back(pos);

		Feature::Instance& feature = s_features[feature_index];
		Item::Handle item_stack = (Item::Handle)feature.payload;
		while (item_stack.valid())
		{
			Item::Handle next_item = Item::unstack(item_stack);
			World::edit().add_item(Random::from_vector(open_pos), next_item);
		}

		Feature::remove(pos);
	}
}

void init_desk(Feature::Instance& feature)
{
	feature.hp = Random::in_range(3, 8);  // health
}

void init_scanner(Feature::Instance& feature)
{
	// TODO: Set up for scanning
}

void damage_basic(Vec3 pos, Damage::Packet const& damage_packet,
                  Material const material, std::string const name)
{
	int const feature_index = find_feature(pos);
	if (Check(feature_index != c_Invalid))
	{
		float const resistance =
			c_Resistances[(int)material][damage_packet.type];
		int const damage_adjusted = (int)(damage_packet.amount * resistance);

		Feature::Instance& feature = s_features[feature_index];
		feature.hp -= damage_adjusted;

		if (feature.hp <= 0)
		{
			Draw::pos_message(pos, "The " + name + " is destroyed!");
			Feature::remove(pos);
		}
		else if (damage_adjusted > 0)
		{
			switch (damage_packet.type)
			{
			case Damage::Basic:
				Draw::pos_message(pos, "The " + name + " is damaged.");
				break;
			case Damage::ToLife:
				Draw::pos_message(pos, "The " + name + " is damaged biologically.");
				DebugBreak("Feature should not be damaged by ToLife damage.");
				break;
			case Damage::Fire:
				Draw::pos_message(pos, "The " + name + " is burned.");
				break;
			case Damage::Acid:
				Draw::pos_message(pos, "The " + name + " is burned by acid.");
				break;
			}
		}
		else
		{
			Draw::pos_message(pos, "The " + name + " is not affected.");
		}
	}
}

void light_torch(Vec3 pos)
{
	int const feature_index = find_feature(pos);
	if (Check(feature_index != c_Invalid))
	{
		Draw::pos_message(pos, "The torch bursts into flames!");
		World::edit().set_terrain(pos, Terrain::TorchLit);

		int trigger = s_features[feature_index].payload;
		if (!is_any_unlit_torch_with_trigger(trigger))
		{
			trigger_all(trigger);
		}
	}
}

bool is_any_unlit_torch_with_trigger(int trigger)
{
	for (const Feature::Instance & feature : s_features)
	{
		if (feature.payload == trigger &&
			World::read().get_terrain(feature.pos) == Terrain::TorchUnlit)
		{
			return true;
		}
	}
	return false;
}

void open_portrait(Vec3 pos)
{
	int const feature_index = find_feature(pos);
	if (Check(feature_index != c_Invalid))
	{
		Draw::pos_message(pos, "The portrait swings open!");
		Feature::remove(pos);
	}
}

void activate_flipendo_button(Vec3 pos)
{
	int const feature_index = find_feature(pos);
	if (Check(feature_index != c_Invalid))
	{
		// only print a message for the one button we cast on
		//  -> other buttons on the same trigger flip silently
		Draw::pos_message(pos, "The button flips.");

		// this Feature is removed when it triggers itself
		trigger_all(s_features[feature_index].payload);
	}
}

void trigger_all(int trigger)
{
	if (trigger == c_Invalid)
	{
		return;
	}

	// search backwards so indexes stay consistant
	for (int i = Util::Size(s_features) - 1; i >= 0; --i)
	{
		if (s_features[i].payload != trigger)
		{
			continue;
		}

		Terrain::Type feature_type = World::read().get_terrain(s_features[i].pos);
		switch (feature_type)
		{
		case Terrain::FlipendoButton:
			trigger_flipendo_button(s_features[i]);
			break;
		case Terrain::SlidingWall:
			trigger_sliding_wall(s_features[i]);
			break;
		case Terrain::Portcullis:
			trigger_portcullis(s_features[i]);
			break;
		case Terrain::ShopSeed:
			trigger_shop_seed(s_features[i]);
			break;
		}
	}
}

void trigger_flipendo_button(Feature::Instance & feature)
{
	Feature::remove(feature.pos, Terrain::Wall);
}

void trigger_sliding_wall(Feature::Instance & feature)
{
	Draw::pos_message(feature.pos, "A wall slides aside!");
	Feature::remove(feature.pos);
}

void trigger_portcullis(Feature::Instance & feature)
{
	Draw::pos_message(feature.pos, "A portcullis opens!");
	Feature::remove(feature.pos);
}

void trigger_shop_seed(Feature::Instance & feature)
{
	Vec3 pos = feature.pos;
	Draw::pos_message(pos, "A shop appears!");
	Feature::remove(pos);

	// TODO: Spawn a real shop
	spawn(pos, Terrain::Chest);
}

} // namespace Feature
