#include "Score.h"

#include "Cloud.h"
#include "Creature.h"
#include "Damage.h"
#include "Debug.h"
#include "Game.h"
#include "Grammar.h"
#include "Inventory.h"
#include "Player.h"
#include "SerializeSaveLoad.h"
#include "Status.h"
#include "VectorUtil.h"

#include <filesystem>

namespace Score
{

//-------------------------------------------------------------------------------------------------
// Data

int constexpr c_ScoreVersionNumber = 0;
int constexpr c_MaxEntries = 20;

static std::vector<Score::Entry> s_high_scores;
static int s_active_row = c_Invalid;
static Entry s_player_entry {};

//-------------------------------------------------------------------------------------------------
// Helper function declarations

void save();
void make_defeated_by_string(std::string& out_string);
int compute_points();

//-------------------------------------------------------------------------------------------------
// Interface

// Helper function
void serialize_scores(ISerializer& s)
{
	int version = c_ScoreVersionNumber;
	s.srz_int(version);

	s.srz_vector_size(s_high_scores, "high scores");
	for (Entry& entry : s_high_scores)
	{
		s.srz_string(entry.name);
		s.srz_string(entry.defeated_by);
		s.srz_value(entry.ending);
		s.srz_int(entry.points);

		// if (version >= 1) ...
	}
}

void init()
{
	s_high_scores.reserve(c_MaxEntries);

	if (std::filesystem::exists("Save/Scores.dat"))
	{
		LoadSerializer s("Save/Scores.dat");
		serialize_scores(s);
	}
}

void clear()
{
	s_active_row = c_Invalid;
}

Entry const& make_player_score()
{
	s_player_entry.name = Player::name();
	make_defeated_by_string(s_player_entry.defeated_by);
	s_player_entry.ending = Ending::Defeated;
	s_player_entry.points = compute_points();

	bool inserted = false;
	for (int row = 0; row < num_scores(); ++row)
	{
		Entry const& entry = s_high_scores[row];
		if (s_player_entry.points > entry.points)
		{
			if (num_scores() == c_MaxEntries)
			{
				s_high_scores.pop_back();
			}

			Util::InsertAt(s_high_scores, row, s_player_entry);
			s_active_row = row;
			inserted = true;
			break;
		}
	}

	if (!inserted && num_scores() < c_MaxEntries)
	{
		s_high_scores.push_back(s_player_entry);
		s_active_row = Util::LastIndex(s_high_scores);
	}
	
	// And save to file.
	save();

	return s_player_entry;
}

int num_scores()
{
	return Util::Size(s_high_scores);
}

int active_row()
{
	return s_active_row;
}

Entry const& read_entry(int rank)
{
	return s_high_scores.at(rank);
}

//-------------------------------------------------------------------------------------------------
// Helper function implementations

void save()
{
	if (!std::filesystem::exists("Save/"))
	{
		std::filesystem::create_directory("Save/");
	}

	SaveSerializer s("Save/Scores.dat");
	serialize_scores(s);
}

void make_defeated_by_string(std::string& out_string)
{
	out_string = "";

	Damage::Cause cause = Player::get_defeated_by();

	if (cause.type == Damage::Cause::Creature)
	{
		Creature::Type const creature_type = (Creature::Type)cause.index;
		assert(Creature::is_valid_type(creature_type));

		if (creature_type == Creature::Player)
		{
			out_string = "... yourself";
		}
		else
		{
			out_string = Grammar::format_name_by_type(creature_type,
				{ .long_name = true, .mode = Grammar::NameParam::Indefinite });
		}
	}
	else if (cause.type == Damage::Cause::Status)
	{
		Status::Index const status = (Status::Index)cause.index;
		char const* predicate = nullptr;
		switch (status)
		{
			case Status::Dancing:
				out_string = "careless dancing";
				break;
			case Status::Burning:
				out_string = "catching on fire";
				break;
			case Status::Venom:
				out_string = "venom";
				break;
			default:
				DebugBreak("Defeated by: unhandled status index.");
				out_string = "an unexpected situation";
				break;
		}
	}
	else if (cause.type == Damage::Cause::Cloud)
	{
		Cloud::Type const cloud = (Cloud::Type)cause.index;
		char const* predicate = nullptr;
		switch (cloud)
		{
			case Cloud::Slime:
				out_string = "acidic slime";
				break;
			default:
				DebugBreak("Defeated by: unhandled cloud type.");
				out_string = "a cloud of errors";
				break;
		}
	}
	else
	{
		DebugBreak("Defeated by: unknown cause.");
		out_string = "the vagaries of fate";
	}
}

int compute_points()
{
	int points = 0;

	points += Player::current_xp();
	points += Player::total_xp_spent();

	int const bean_slot = Inventory::read().find_first_item(Item::Type::BBBean);
	if (bean_slot != c_Invalid)
	{
		points += Inventory::read().peek_item(bean_slot).stack_height();
	}

	return points;
}

} // namespace Score