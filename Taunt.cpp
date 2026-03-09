#include "Taunt.h"

#include "Creature.h"
#include "Debug.h"
#include "Draw.h"
#include "Grammar.h"
#include "Player.h"
#include "Random.h"
#include "Serialize.h"
#include "Status.h"

#include <cctype>
#include <format>

namespace Taunt
{

//-------------------------------------------------------------------------------------------------
// Data

using TauntList = std::vector<Taunt::Data>;

static TauntList s_taunts [Creature::Count];

//-------------------------------------------------------------------------------------------------
// Helper declarations

void mumble_in_place(std::string& str, int mumble_amount);

//-------------------------------------------------------------------------------------------------
// Interface

void init()
{
	// Taunt: text, presentation, condition, subtype, repeatable
	// Format strings: {0} PlayerName

	s_taunts[Creature::ColinCreevy_0] =
	{
		Taunt::Data{.text="No way, are you {0}?",
			.condition=AnyTime, .format=true},
		Taunt::Data{.text="hops up and down",
			.presentation=Emote, .condition=AnyTime},
	};

	s_taunts[Creature::Neville_0] =
	{
		Taunt::Data{.text="Have you seen my toad?"},
		Taunt::Data{.text="Do you feel like you've forgotten something?"},
		Taunt::Data{.text="You're sneaking out again, aren't you?"},
		Taunt::Data{.text="I won't let you!  I'll fight you!"},

		Taunt::Data{.text="looks miserable, but determined",
			.presentation=Emote, .condition=Hurt},
	};

	s_taunts[Creature::SallyAnne_0] =
	{
		Taunt::Data{.text="looks distant",
			.presentation=Emote, .condition=AnyTime},
		Taunt::Data{.text="looks thoughtful",
			.presentation=Emote, .condition=AnyTime},
	};

	s_taunts[Creature::Harry_1] =
	{	
		Taunt::Data{.text="I'm Harry.  Just Harry.",
			.condition=Greeting},
		Taunt::Data{.text="My scar hurts.",
			.rarity=2},
		Taunt::Data{.text="I'm a what?",
			.rarity=5},
		Taunt::Data{.text="I mean every word I ever say, ever.",
			.rarity=10},
		Taunt::Data{.text="Because I'm Harry Potter.",
			.condition=FollowUp},
		Taunt::Data{.text="Are you all right?",
			.condition=Winning},
		Taunt::Data{.text="I'm Harry Potter, Harry Potter, ooh!",
			.condition=Winning, .rarity=10},
		Taunt::Data{.text="Harry Potter, Harry Potter, YEAH!",
			.condition=FollowUp},
		Taunt::Data{.text="Harry Potter, Harry Potter, that's me!",
			.condition=FollowUp},
		// Angst, angst, angst.
	};

	s_taunts[Creature::Malfoy_1] =
	{	
		Taunt::Data{.text="Hello, {0}.",
			.condition=Greeting, .format=true},
		Taunt::Data{.text="Where do you think you're going?"},

		Taunt::Data{.text="sneers scornfully",
			.presentation=Emote, .rarity=2},
		Taunt::Data{.text="sneers contemptuously",
			.presentation=Emote, .rarity=2},
		Taunt::Data{.text="sneers condescendingly",
			.presentation=Emote, .rarity=2},
		Taunt::Data{.text="sneers disdainfully",
			.presentation=Emote, .rarity=2},
		Taunt::Data{.text="sneers supercilliously",
			.presentation=Emote, .rarity=4},
		Taunt::Data{.text="sneers insolently",
			.presentation=Emote, .rarity=3},
		Taunt::Data{.text="sneers derisively",
			.presentation=Emote, .rarity=2},
		Taunt::Data{.text="sneers mockingly",
			.presentation=Emote, .rarity=3},
		Taunt::Data{.text="sneers spitefully",
			.presentation=Emote, .rarity=3},
		Taunt::Data{.text="sneers dismissively",
			.presentation=Emote, .rarity=4},
		Taunt::Data{.text="sneers disgustedly",
			.presentation=Emote, .rarity=4},
		Taunt::Data{.text="sneers imperiously",
			.presentation=Emote, .rarity=4},
		Taunt::Data{.text="sneers resentfully",
			.presentation=Emote, .rarity=4},
		Taunt::Data{.text="sneers overbearingly",
			.presentation=Emote, .rarity=4},
		Taunt::Data{.text="sneers smugly",
			.presentation=Emote, .rarity=3},
		Taunt::Data{.text="sneers smirkingly",
			.presentation=Emote, .rarity=5},

		Taunt::Data{.text="smirks unpleasantly",
			.presentation=Emote, .rarity=3},
		Taunt::Data{.text="smirks deprecatingly",
			.presentation=Emote, .rarity=3},
		Taunt::Data{.text="smirks judgementally",
			.presentation=Emote, .rarity=3},
		Taunt::Data{.text="smirks disparagingly",
			.presentation=Emote, .rarity=3},
		Taunt::Data{.text="smirks insultingly",
			.presentation=Emote, .rarity=3},
		Taunt::Data{.text="smirks self-righteously",
			.presentation=Emote, .rarity=4},
		Taunt::Data{.text="smirks arrongantly",
			.presentation=Emote, .rarity=3},
		Taunt::Data{.text="smirks patronizingly",
			.presentation=Emote, .rarity=3},
		Taunt::Data{.text="smirks haughtily",
			.presentation=Emote, .rarity=3},
		Taunt::Data{.text="smirks unsympathetically",
			.presentation=Emote, .rarity=3},
		Taunt::Data{.text="smirks snobbishly",
			.presentation=Emote, .rarity=3},
		Taunt::Data{.text="smirks mockingly",
			.presentation=Emote, .rarity=3},
		Taunt::Data{.text="smirks smarmily",
			.presentation=Emote, .rarity=4},
		Taunt::Data{.text="smirks priggishly",
			.presentation=Emote, .rarity=3},
		Taunt::Data{.text="smirks snidely",
			.presentation=Emote, .rarity=3},
		Taunt::Data{.text="smirks sneeringly",
			.presentation=Emote, .rarity=5},

		Taunt::Data{.text="Scared?",
			.condition=Winning},
		Taunt::Data{.text="smirks sadistically",
			.presentation=Emote, .condition=Winning},
		Taunt::Data{.text="How dare you?",
			.condition=Hurt},
		Taunt::Data{.text="Just wait until my father hears.",
			.condition=Hurt},
		Taunt::Data{.text="Practicing for the ballet?",
			.condition=PlayerStatus, .subtype=Status::Dancing},
		Taunt::Data{.text="Think this is funny, do you?",
			.condition=HasStatus, .subtype=Status::Tickled},
		Taunt::Data{.text="You'll pay for this!",
			.condition=HasStatus, .subtype=Status::Dancing},
	};

	s_taunts[Creature::Hermione_2] =
	{
		Taunt::Data{.text="I'm Hermione Granger.  And you are?",
			.condition=Greeting},
		Taunt::Data{.text="It's for your own good, you know."},
		Taunt::Data{.text="Don't make me late for class."},
		Taunt::Data{.text="Any more bright ideas to get us killed?",
			.condition=Winning},
		Taunt::Data{.text="Or worse, expelled.",
			.condition=FollowUp},
		Taunt::Data{.text="I'm really sorry about this.",
			.condition=AttackSpell, .subtype=Spell::LacarnumInflamare},
		Taunt::Data{.text="You're quite the hellion today.",
			.condition=Hurt, .rarity=5},
		Taunt::Data{.text="Are you sure that's a real spell?",
			.condition=PlayerMiscast},
		Taunt::Data{.text="You ought to study more.",
			.condition=PlayerMiscast},
		Taunt::Data{.text="You're going to take someone's eye out.",
			.condition=PlayerMiscast},
		Taunt::Data{.text="It's flip-END-oh, not flippen-DOH.",
			.condition=PlayerMiscast, .subtype=Spell::Flipendo},
		Taunt::Data{.text="It's ver-MILL-ious, not vermi-LEE-us.",
			.condition=PlayerMiscast, .subtype=Spell::Vermillious},
		Taunt::Data{.text="It's rictu-SEM-pra, not rictoosem-PRA.",
			.condition=PlayerMiscast, .subtype=Spell::Rictusempra},
		Taunt::Data{.text="Stop that this instant.",
			.condition=HasStatus, .subtype=Status::Tickled},
	};

	s_taunts[Creature::Ron_2] =
	{
		Taunt::Data{.text="I'm Ron Weasley, by the way."},
		Taunt::Data{.text="Blimey!"},
		Taunt::Data{.text="Are you mental?"},
		Taunt::Data{.text="Take this, cauldron-bum!",
			.condition=AttackSpell, .rarity=5},
		Taunt::Data{.text="Oh, bloody hell!",
			.condition=Hurt},
		Taunt::Data{.text="Dragon bogeys!",
			.condition=HasStatus, .subtype=Status::Dancing, .rarity=5},
		Taunt::Data{.text="You need to sort our your priorities.",
			.condition=Hurt},
		Taunt::Data{.text="You git!",
			.condition=Hurt},
		Taunt::Data{.text="Ron, Ron, Ron Weasley!",
			.condition=Winning, .rarity=6},
		// "Excuse me, are you the imprint of a departed soul?"
	};

	// Crabbe_3,
	// Goyle_3,
	// Harry_4,
	// Cedric_4,
	// Fleur_4,
	// Krum_5,
	// Neville_5,
	// Ginny_5,
	// Luna_5,

	s_taunts[Creature::Gnome] =
	{
		Taunt::Data{.text="Gerronk!", .condition=AnyTime, .repeat=true},
		Taunt::Data{.text="Buurrf!", .condition=AnyTime, .repeat=true},
		Taunt::Data{.text="Gweeonken!", .condition=AnyTime, .repeat=true},
		Taunt::Data{.text="Berruff!", .condition=AnyTime, .repeat=true},
		Taunt::Data{.text="Hyehheh!", .condition=AnyTime, .repeat=true},
		Taunt::Data{.text="Ehehehe!", .condition=AnyTime, .repeat=true},
		Taunt::Data{.text="Gwork!", .condition=AnyTime, .repeat=true},
		Taunt::Data{.text="Gursh bur murfen!", .condition=AnyTime, .repeat=true},
		Taunt::Data{.text="Tur ukka noka!", .condition=AnyTime, .repeat=true},
		Taunt::Data{.text="Blurffen hoke!", .condition=AnyTime, .repeat=true},
		Taunt::Data{.text="Donk donk ptonk!", .condition=AnyTime, .repeat=true},
		Taunt::Data{.text="Wusha heefen makka!", .condition=AnyTime, .repeat=true},
		Taunt::Data{.text="Mmrnmhrm.", .condition=AnyTime, .repeat=true},
		Taunt::Data{.text="Jorken torpus.", .condition=AnyTime, .repeat=true},
		Taunt::Data{.text="Hruff uh gwell.", .condition=AnyTime, .repeat=true},
		Taunt::Data{.text="Bee yehh...", .condition=AnyTime, .repeat=true},

		Taunt::Data{.text="rubs its head",
			.presentation=Emote, .condition=AnyTime, .repeat=true},
		Taunt::Data{.text="rubs its hands together",
			.presentation=Emote, .condition=AnyTime, .repeat=true},
		Taunt::Data{.text="sniffs the ground",
			.presentation=Emote, .condition=AnyTime, .repeat=true},
		Taunt::Data{.text="sniffs the air",
			.presentation=Emote, .condition=AnyTime, .repeat=true},
		Taunt::Data{.text="makes potato sounds",
			.presentation=Emote, .condition=AnyTime, .repeat=true},
		Taunt::Data{.text="looks around",
			.presentation=Emote, .condition=AnyTime, .repeat=true},
		Taunt::Data{.text="hops excitedly",
			.presentation=Emote, .condition=AnyTime, .repeat=true},
	};

	s_taunts[Creature::Streeler] =
	{
		Taunt::Data{.text="makes a squelching sound",
			.presentation=Emote, .condition=AnyTime, .repeat=true},
		Taunt::Data{.text="wiggles its eye-stalks",
			.presentation=Emote, .condition=AnyTime, .repeat=true},
		Taunt::Data{.text="croodles to itself",
			.presentation=Emote, .condition=AnyTime, .repeat=true},
	};

	s_taunts[Creature::FireCrab] =
	{
		Taunt::Data{.text="growls",
			.presentation=Emote, .condition=AnyTime, .repeat=true},
		Taunt::Data{.text="waggles its claws",
			.presentation=Emote, .condition=AnyTime, .repeat=true},
		Taunt::Data{.text="shakes its rear end",
			.presentation=Emote, .condition=AnyTime, .repeat=true},
	};

	s_taunts[Creature::HarryTheHufflepuff_1] = 
	{
		Taunt::Data{.text="This is too much work."},
		Taunt::Data{.text="Do we need to do this?"},
		Taunt::Data{.text="Do you know anything about house-elves?"},
		Taunt::Data{.text="Robes are great.  No need for trousers.",
			.rarity=3},
		Taunt::Data{.text="Could you make this a little easier?",
			.condition=Hurt},
		Taunt::Data{.text="This is more effort than it's worth.",
			.condition=Hurt},
		Taunt::Data{.text="Being bullied is easier than resisting.",
			.condition=Hurt},
		Taunt::Data{.text="You're waving your wand too much.",
			.condition=PlayerMiscast},
		Taunt::Data{.text="Try doing it more slowly.",
			.condition=PlayerMiscast},
		Taunt::Data{.text="Time for my famous Harry-Kari strategy.",
			.condition=AttackSpell},
	};
}

void clear()
{
	for (TauntList& list : s_taunts)
	{
		for (Taunt::Data& taunt : list)
		{
			taunt.uses = 0;
		}
	}
}

void Taunt::Data::serialize(ISerializer& s)
{
	// Only serialize the runtime data.
	s.srz_int(uses);
}

void serialize(ISerializer& s)
{
	for (int i = 0; i < Creature::Count; ++i)
	{
		int const real_size = Util::Size(s_taunts[i]);
		int num = Util::Size(s_taunts[i]);
		s.srz_int(num);

		for (int t = 0;
			t < real_size && t < num;
			++t)
		{
			s_taunts[i].at(t).serialize(s);
		}
	}
}

void find_taunts(Creature::Handle taunter, Condition condition, int subtype,
	IntTempList& out_taunts)
{
	Creature::Type const type = taunter.type();
	if (type < 0 || type > Creature::Count)
	{
		DebugBreak("Taunts for invalid creature type.");
		return;
	}

	TauntList const& taunt_list = s_taunts[type];
	out_taunts.reserve(taunt_list.size());

	for (int i = 0; i < taunt_list.size(); ++i)
	{
		Taunt::Data const& taunt = taunt_list.at(i);

		if (taunt.condition == condition &&
			(taunt.repeat || taunt.uses == 0) &&
			(taunt.subtype == c_Invalid || taunt.subtype == subtype) &&
			(taunt.rarity <= 1 || Random::one_in(taunt.rarity)))
		{
			out_taunts.push_back(i);
		}
	}
}

void find_status_taunts(Creature::Handle taunter, Creature::Handle target,
	IntTempList& out_taunts)
{
	Creature::Type const type = taunter.type();
	if (type < 0 || type > Creature::Count)
	{
		DebugBreak("Taunts for invalid creature type.");
		return;
	}

	TauntList const& taunt_list = s_taunts[type];
	out_taunts.reserve(taunt_list.size());

	for (int i = 0; i < taunt_list.size(); ++i)
	{
		Taunt::Data const& taunt = taunt_list.at(i);

		if (taunt.condition == PlayerStatus &&
			target.has_status((Status::Index)taunt.subtype) &&
			(taunt.repeat || taunt.uses == 0) &&
			(taunt.rarity <= 1 || Random::one_in(taunt.rarity)))
		{
			out_taunts.push_back(i);
		}

		else if (taunt.condition == HasStatus &&
			taunter.has_status((Status::Index)taunt.subtype) &&
			(taunt.repeat || taunt.uses == 0) &&
			(taunt.rarity <= 1 || Random::one_in(taunt.rarity)))
		{
			out_taunts.push_back(i);
		}
	}
}

int find_followup(Creature::Handle taunter, int last_taunt)
{
	Creature::Type const type = taunter.type();
	if (type < 0 || type > Creature::Count)
	{
		DebugBreak("Taunts for invalid creature type.");
		return c_Invalid;
	}

	TauntList const& taunt_list = s_taunts[type];
	int followup = last_taunt + 1;
	if (Util::IsValidIndex(taunt_list, followup))
	{
		Taunt::Data const& taunt = taunt_list[followup];
		if (taunt.condition == FollowUp &&
			(taunt.repeat || taunt.uses == 0) &&
			(taunt.rarity <= 1 || Random::one_in(taunt.rarity)))
		{
			return followup;
		}
	}
	
	return c_Invalid;
}

void say_taunt(Creature::Handle taunter, int taunt_id)
{
	Creature::Type const type = taunter.type();
	if (type < 0 || type > Creature::Count)
	{
		DebugBreak("Taunting with invalid creature type.");
		return;
	}

	TauntList& taunt_list = s_taunts[type];
	Taunt::Data& taunt = taunt_list.at(taunt_id);

	if (Check(taunt.repeat || taunt.uses == 0, "Reusing non-repeat taunt."))
	{
		++taunt.uses;

		char const* message = taunt.text;
		std::string formatted;

		if (taunt.format)
		{
			formatted = std::vformat(message, std::make_format_args(Player::name()));
			message = formatted.c_str();
		}

		if (taunt.presentation == Presentation::Say)
		{
			if (taunter.has_status(Status::TongueTied))
			{
				formatted = message;
				mumble_in_place(formatted, taunter.status_severity(Status::TongueTied));
				message = formatted.c_str();
			}

			// We don't do "creature_message" because we only call this function if it's
			// appropriate to display the message.
			Draw::add_message(std::format("{} says, \"{}\"", Grammar::You(taunter), message));
		}
		else if (taunt.presentation == Presentation::Emote)
		{
			Draw::add_message(std::format("{} {}.", Grammar::You(taunter), message));
		}
	}
}

//-------------------------------------------------------------------------------------------------
// Helper implementations

void mumble_in_place(std::string& str, int mumble_amount)
{
	char last_old = 0;
	char last_new = 0;

	for (char& c : str)
	{
		if (isalpha(c) && Random::in_range(0,15) < mumble_amount)
		{
			bool const was_upper = isupper(c);
			c = tolower(c);

			char old = c;
			
			if (c == last_old)
			{
				c = last_new;
			}
			else
			{
				switch (c)
				{
					case 'a':
					case 'i':
					case 'o':
					case 'u':
						switch (Random::in_range(0,2))
						{
							case 0: c = 'a'; break;
							case 1: c = 'o'; break;
							case 2: c = 'u'; break;
						}
						break;

					case 'b':
					case 'd':
					case 'f':
					case 'p':
					case 't':
					case 'v':
					case 'w':
						switch (Random::in_range(0,6))
						{
							case 0: c = 'b'; break;
							case 1: c = 'd'; break;
							case 2: c = 'f'; break;
							case 3: c = 'p'; break;
							case 4: c = 't'; break;
							case 5: c = 'v'; break;
							case 6: c = 'w'; break;
						}
						break;

					case 'g':
					case 'k':
						switch (Random::in_range(0,3))
						{
							case 0: c = 'g'; break;
							case 1: c = 'k'; break;
							case 2: c = 'h'; break;
							case 3: c = 'm'; break;
						}
						break;

					case 'j':
					case 'z':
						switch (Random::in_range(0,3))
						{
							case 0: c = 'j'; break;
							case 1: c = 'z'; break;
							case 2: c = 's'; break;
							case 3: c = 'm'; break;
						}
						break;

					case 'l':
					case 'm':
					case 'n':
						switch (Random::in_range(0,3))
						{
							case 0: c = 'l'; break;
							case 1: c = 'm'; break;
							case 2: c = 'n'; break;
							case 3: c = 'h'; break;
						}
						break;

					case 'r':
						switch (Random::in_range(0,2))
						{
							case 0: c = 'r'; break;
							case 1: c = 'w'; break;
							case 2: c = 'm'; break;
						}
						break;

					default: // e, h, q, y
						break;
				}
			}

			last_old = old;
			last_new = c;

			if (was_upper)
			{
				c = toupper(c);
			}
		}
	}
}

} // namespace Taunt
