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
			.format=true},
		Taunt::Data{.text="Wow, you're famous!"},
		Taunt::Data{.text="Let me take your picture!"},

		Taunt::Data{.text="hops up and down",
			.presentation=Emote},
	};

	s_taunts[Creature::Neville_0] =
	{
		Taunt::Data{.text="Have you seen my toad?"},
		Taunt::Data{.text="Have I forgotten something?"},
		Taunt::Data{.text="You're sneaking out again, aren't you?"},
		Taunt::Data{.text="I won't let you!  I'll fight you!",
			.condition=FollowUp},

		Taunt::Data{.text="looks miserable, but determined",
			.presentation=Emote, .condition=Losing},
		Taunt::Data{.text="I'm not giving up!",
			.condition=Losing},
		Taunt::Data{.text="You told me to stand up for myself!",
			.condition=Losing},
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
			.rarity=5},
		Taunt::Data{.text="I'm a what?",
			.rarity=10},
		Taunt::Data{.text="I mean every word I ever say, ever.",
			.rarity=30},
		Taunt::Data{.text="Because I'm Harry Potter.",
			.condition=FollowUp},
		Taunt::Data{.text="Are you all right?",
			.condition=Winning},
		Taunt::Data{.text="I'm Harry Potter, Harry Potter, ooh!",
			.condition=Winning, .rarity=30},
		Taunt::Data{.text="Harry Potter, Harry Potter, YEAH!",
			.condition=FollowUp},
		Taunt::Data{.text="Harry Potter, Harry Potter, that's me!",
			.condition=FollowUp},
	};

	s_taunts[Creature::Malfoy_1] =
	{	
		Taunt::Data{.text="Hello, {0}.",
			.condition=Greeting, .format=true},
		Taunt::Data{.text="Where do you think you're going?"},

		Taunt::Data{.text="sneers scornfully",
			.presentation=Emote, .rarity=4},
		Taunt::Data{.text="sneers contemptuously",
			.presentation=Emote, .rarity=4},
		Taunt::Data{.text="sneers condescendingly",
			.presentation=Emote, .rarity=4},
		Taunt::Data{.text="sneers disdainfully",
			.presentation=Emote, .rarity=4},
		Taunt::Data{.text="sneers supercilliously",
			.presentation=Emote, .rarity=6},
		Taunt::Data{.text="sneers insolently",
			.presentation=Emote, .rarity=5},
		Taunt::Data{.text="sneers derisively",
			.presentation=Emote, .rarity=4},
		Taunt::Data{.text="sneers mockingly",
			.presentation=Emote, .rarity=5},
		Taunt::Data{.text="sneers spitefully",
			.presentation=Emote, .rarity=5},
		Taunt::Data{.text="sneers dismissively",
			.presentation=Emote, .rarity=6},
		Taunt::Data{.text="sneers disgustedly",
			.presentation=Emote, .rarity=6},
		Taunt::Data{.text="sneers imperiously",
			.presentation=Emote, .rarity=6},
		Taunt::Data{.text="sneers resentfully",
			.presentation=Emote, .rarity=6},
		Taunt::Data{.text="sneers overbearingly",
			.presentation=Emote, .rarity=6},
		Taunt::Data{.text="sneers smugly",
			.presentation=Emote, .rarity=5},
		Taunt::Data{.text="sneers smirkingly",
			.presentation=Emote, .rarity=7},

		Taunt::Data{.text="smirks unpleasantly",
			.presentation=Emote, .rarity=5},
		Taunt::Data{.text="smirks deprecatingly",
			.presentation=Emote, .rarity=5},
		Taunt::Data{.text="smirks judgementally",
			.presentation=Emote, .rarity=5},
		Taunt::Data{.text="smirks disparagingly",
			.presentation=Emote, .rarity=5},
		Taunt::Data{.text="smirks insultingly",
			.presentation=Emote, .rarity=5},
		Taunt::Data{.text="smirks self-righteously",
			.presentation=Emote, .rarity=6},
		Taunt::Data{.text="smirks arrongantly",
			.presentation=Emote, .rarity=5},
		Taunt::Data{.text="smirks patronizingly",
			.presentation=Emote, .rarity=5},
		Taunt::Data{.text="smirks haughtily",
			.presentation=Emote, .rarity=5},
		Taunt::Data{.text="smirks unsympathetically",
			.presentation=Emote, .rarity=5},
		Taunt::Data{.text="smirks snobbishly",
			.presentation=Emote, .rarity=5},
		Taunt::Data{.text="smirks mockingly",
			.presentation=Emote, .rarity=5},
		Taunt::Data{.text="smirks smarmily",
			.presentation=Emote, .rarity=6},
		Taunt::Data{.text="smirks priggishly",
			.presentation=Emote, .rarity=5},
		Taunt::Data{.text="smirks snidely",
			.presentation=Emote, .rarity=5},
		Taunt::Data{.text="smirks sneeringly",
			.presentation=Emote, .rarity=7},

		Taunt::Data{.text="Scared?",
			.condition=Winning},
		Taunt::Data{.text="Haha!  I've got you now.",
			.condition=Winning},
		Taunt::Data{.text="smirks sadistically",
			.presentation=Emote, .condition=Winning},
		Taunt::Data{.text="How dare you?",
			.condition=Losing},
		Taunt::Data{.text="Just wait until my father hears.",
			.condition=Losing},
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
			.condition=Losing, .rarity=20},
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
			.condition=AttackSpell, .rarity=8},
		Taunt::Data{.text="Dragon bogeys!",
			.condition=HasStatus, .subtype=Status::Dancing, .rarity=8},
		Taunt::Data{.text="Oh, bloody hell!",
			.condition=Losing},
		Taunt::Data{.text="You need to sort our your priorities.",
			.condition=Losing},
		Taunt::Data{.text="You git!",
			.condition=Losing},
		Taunt::Data{.text="Ron, Ron, Ron Weasley!",
			.condition=Winning, .rarity=8},

		// "Excuse me, are you the imprint of a departed soul?"
	};

	s_taunts[Creature::Crabbe_3] =
	{
		Taunt::Data{.text="calls out",
			.presentation=Emote, .condition=Winning},
		Taunt::Data{.text="chortles",
			.presentation=Emote, .condition=Winning},
		Taunt::Data{.text="cheers",
			.presentation=Emote, .condition=Winning},
		Taunt::Data{.text="cackles",
			.presentation=Emote, .condition=Winning},
		Taunt::Data{.text="cringes",
			.presentation=Emote, .condition=Losing},
		Taunt::Data{.text="cowers",
			.presentation=Emote, .condition=Losing},
		Taunt::Data{.text="cracks up",
			.presentation=Emote, .condition=HasStatus, .subtype=Status::Tickled},
		Taunt::Data{.text="looks crispy",
			.presentation=Emote, .condition=HasStatus, .subtype=Status::Burning},
		Taunt::Data{.text="does the can-can",
			.presentation=Emote, .condition=HasStatus, .subtype=Status::Dancing},
		Taunt::Data{.text="does the cha-cha",
			.presentation=Emote, .condition=HasStatus, .subtype=Status::Dancing},
	};
	
	s_taunts[Creature::Goyle_3] =
	{
		Taunt::Data{.text="grunts",
			.presentation=Emote, .condition=Greeting},
		Taunt::Data{.text="grins",
			.presentation=Emote, .condition=Winning},
		Taunt::Data{.text="gloats",
			.presentation=Emote, .condition=Winning},
		Taunt::Data{.text="grimaces",
			.presentation=Emote, .condition=Losing},
		Taunt::Data{.text="glowers",
			.presentation=Emote, .condition=Losing},
		Taunt::Data{.text="groans",
			.presentation=Emote, .condition=Losing},
		Taunt::Data{.text="giggles",
			.presentation=Emote, .condition=HasStatus, .subtype=Status::Tickled},
		Taunt::Data{.text="gasps",
			.presentation=Emote, .condition=HasStatus, .subtype=Status::Burning},
		Taunt::Data{.text="gallops and gambols",
			.presentation=Emote, .condition=HasStatus, .subtype=Status::Dancing},
		Taunt::Data{.text="dances Gangnam Style",
			.presentation=Emote, .condition=HasStatus, .subtype=Status::Dancing, .rarity=20},
	};

	s_taunts[Creature::Harry_4] =
	{
		Taunt::Data{.text="None of this is my fault."},
		Taunt::Data{.text="Angst, angst, angst.",
			.rarity=40},
		Taunt::Data{.text="I feel cranky and pubescent today and I don't know why.",
			.rarity=30},
		Taunt::Data{.text="I'm going to take it out on people I like.",
			.condition=FollowUp},
	};

	s_taunts[Creature::Cedric_4] =
	{
		Taunt::Data{.text="gives you a smile and draws his wand.",
			.presentation=Emote, .condition=Greeting},
		Taunt::Data{.text="sparkles handsomely",
			.presentation=Emote, .condition=AnyTime, .rarity=20},
		Taunt::Data{.text="I'm the real Hogwarts champion!"},
		Taunt::Data{.text="strikes a pose",
			.presentation=Emote, .condition=Winning},
	};

	s_taunts[Creature::Fleur_4] =
	{
		Taunt::Data{.text="Bonsoir, {0}!",
			.condition=Greeting, .format=true},
		Taunt::Data{.text="Is zis normal, in 'Ogwarts?"},
		Taunt::Data{.text="Why is all zis 'appening?"},
		Taunt::Data{.text="C'est impossible!",
			.condition=Losing},
		Taunt::Data{.text="Evidently you are too young for zis.",
			.condition=Winning},
		Taunt::Data{.text="Ooh la la, zis ees not right!",
			.condition=HasStatus, .subtype=Status::Burning},
		Taunt::Data{.text="Ah, non!  Ma robe!",
			.condition=HasStatus, .subtype=Status::Burning},
		Taunt::Data{.text="It iz vairy funny, non?",
			.condition=PlayerStatus, .subtype=Status::Tickled},
		Taunt::Data{.text="'Aving a leetle trouble?",
			.condition=PlayerMiscast},
		Taunt::Data{.text="Are ze spells too 'ard for you?",
			.condition=PlayerMiscast},
		Taunt::Data{.text="Ze speeking is deeficult for you?",
			.condition=PlayerStatus, .subtype=Status::TongueTied},
		Taunt::Data{.text="Oh deear, 'ave you fallen down?",
			.condition=PlayerStatus, .subtype=Status::Prone},

		Taunt::Data{.text="smiles proudly",
			.presentation=Emote, .condition=Winning, .rarity=4},
		Taunt::Data{.text="pouts haughtily",
			.presentation=Emote, .condition=Losing, .rarity=4},
		Taunt::Data{.text="twirls elegantly",
			.presentation=Emote, .condition=HasStatus, .subtype=Status::Dancing},
		Taunt::Data{.text="pirouettes",
			.presentation=Emote, .condition=HasStatus, .subtype=Status::Dancing},
		Taunt::Data{.text="giggles irresistably",
			.presentation=Emote, .condition=HasStatus, .subtype=Status::Tickled},
		Taunt::Data{.text="laughs uncontrollably",
			.presentation=Emote, .condition=HasStatus, .subtype=Status::Tickled},
		Taunt::Data{.text="makes incoherent noises",
			.presentation=Emote, .condition=HasStatus, .subtype=Status::TongueTied},
	};

	// Krum_5,
	// Neville_5,
	// Ginny_5,
	// Luna_5,

	s_taunts[Creature::MarySue] =
	{
		Taunt::Data{.text="What are YOU doing here?",
			.condition=Greeting},

		Taunt::Data{.text="I'm the youngest witch ever to graduate from Hogwarts."},
		Taunt::Data{.text="I'm the REAL chosen one."},
		Taunt::Data{.text="Don't get in the way of my destiny."},
		Taunt::Data{.text="You know NOTHING of true love."},
		Taunt::Data{.text="The Dark Lord belongs to ME.",
			.rarity=20},
		Taunt::Data{.text="Harry Potter belongs to ME.",
			.rarity=20},
		Taunt::Data{.text="Draco Malfoy belongs to ME.",
			.rarity=40},
		Taunt::Data{.text="tosses her luxurious auburn hair",
			.presentation=Emote},

		Taunt::Data{.text="You're ruining my story!",
			.condition=Losing},
		Taunt::Data{.text="This isn't over yet!",
			.condition=Losing},
		Taunt::Data{.text="What do you think you're doing?",
			.condition=Losing},
		Taunt::Data{.text="What's wrong with your hair?",
			.condition=Losing},
		Taunt::Data{.text="Nothing can stop me!",
			.condition=Losing},
		Taunt::Data{.text="narrows her deep violet eyes",
			.presentation=Emote, .condition=Losing},

		Taunt::Data{.text="You're just not beautiful enough.",
			.condition=Winning},
		Taunt::Data{.text="You'll never defeat me!",
			.condition=Winning},
		Taunt::Data{.text="Crossing me was your greatest mistake!",
			.condition=Winning},
		Taunt::Data{.text="Get ready to lose!",
			.condition=Winning},
		Taunt::Data{.text="You're a walking disaster, {0}.",
			.condition=Winning, .format=true},

		Taunt::Data{.text="Dance, puppet!",
			.condition=PlayerStatus, .subtype=Status::Dancing},

		Taunt::Data{.text="Mwahahaha!",
			.condition=PlayerStatus, .subtype=Status::Batty},
		Taunt::Data{.text="Yes!  Fly, my pretties!",
			.condition=PlayerStatus, .subtype=Status::Batty},
		Taunt::Data{.text="Onwards, the swarm!",
			.condition=PlayerStatus, .subtype=Status::Batty},
		Taunt::Data{.text="Now the darkness surrounds you!",
			.condition=PlayerStatus, .subtype=Status::Batty},

		Taunt::Data{.text="You are helpless before me!",
			.condition=PlayerStatus, .subtype=Status::Tickled},
		Taunt::Data{.text="What a pathetic creature you are.",
			.condition=PlayerStatus, .subtype=Status::Tickled},
		Taunt::Data{.text="I don't like the way you laugh.",
			.condition=PlayerStatus, .subtype=Status::Tickled},
		Taunt::Data{.text="I don't like the way you look.",
			.condition=PlayerStatus, .subtype=Status::Tickled},
		Taunt::Data{.text="Bow down!  Grovel!",
			.condition=PlayerStatus, .subtype=Status::Tickled},
	};

	s_taunts[Creature::Gnome] =
	{
		Taunt::Data{.text="Gerronk!", .repeat=true},
		Taunt::Data{.text="Buurrf!", .repeat=true},
		Taunt::Data{.text="Gweeonken!", .repeat=true},
		Taunt::Data{.text="Berruff!", .repeat=true},
		Taunt::Data{.text="Hyehheh!", .repeat=true},
		Taunt::Data{.text="Ehehehe!", .repeat=true},
		Taunt::Data{.text="Gwork!", .repeat=true},
		Taunt::Data{.text="Gursh bur murfen!", .repeat=true},
		Taunt::Data{.text="Tur ukka noka!", .repeat=true},
		Taunt::Data{.text="Blurffen hoke!", .repeat=true},
		Taunt::Data{.text="Donk donk ptonk!", .repeat=true},
		Taunt::Data{.text="Wusha heefen makka!", .repeat=true},
		Taunt::Data{.text="Burr hur hur!", .repeat=true},
		Taunt::Data{.text="Hurrgg yurg!", .repeat=true},
		Taunt::Data{.text="Doshem ga burf!", .repeat=true},
		Taunt::Data{.text="Hursh ne murbur!", .repeat=true},
		Taunt::Data{.text="Urp bwaaa!", .repeat=true},
		Taunt::Data{.text="Gee plurka!", .repeat=true},
		Taunt::Data{.text="Mmrnmhrm.", .repeat=true},
		Taunt::Data{.text="Jorken torpus.", .repeat=true},
		Taunt::Data{.text="Hruff uh gwell.", .repeat=true},
		Taunt::Data{.text="Bursh nossken.", .repeat=true},
		Taunt::Data{.text="Wrrmmm...", .repeat=true},
		Taunt::Data{.text="Bee yehh...", .repeat=true},
		Taunt::Data{.text="Mugga fwaa...", .repeat=true},
		Taunt::Data{.text="Burrump.", .repeat=true},
		Taunt::Data{.text="Roshum grem.", .repeat=true},
		Taunt::Data{.text="Bree urkum.", .repeat=true},
		Taunt::Data{.text="I've got gnome more to say.", .rarity=100},

		Taunt::Data{.text="rubs its head", .presentation=Emote, .repeat=true},
		Taunt::Data{.text="rubs its hands together", .presentation=Emote, .repeat=true},
		Taunt::Data{.text="sniffs the ground", .presentation=Emote, .repeat=true},
		Taunt::Data{.text="sniffs the air", .presentation=Emote, .repeat=true},
		Taunt::Data{.text="makes potato sounds", .presentation=Emote, .repeat=true},
		Taunt::Data{.text="looks around", .presentation=Emote, .repeat=true},
		Taunt::Data{.text="hops excitedly", .presentation=Emote, .repeat=true},
	};

	s_taunts[Creature::Streeler] =
	{
		Taunt::Data{.text="squeals",
			.presentation=Emote, .condition=Greeting, .repeat=true},
		Taunt::Data{.text="makes a squelching sound",
			.presentation=Emote, .repeat=true},
		Taunt::Data{.text="wiggles its eye-stalks",
			.presentation=Emote, .repeat=true},
		Taunt::Data{.text="burbles to itself",
			.presentation=Emote, .repeat=true},
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

	s_taunts[Creature::BigFireCrab] =
	{
		Taunt::Data{.text="taps its heavy claws",
			.presentation=Emote, .repeat=true},
		Taunt::Data{.text="rumbles deeply",
			.presentation=Emote, .repeat=true},
		Taunt::Data{.text="gives a throaty growl",
			.presentation=Emote, .repeat=true},
		Taunt::Data{.text="growls angrily",
			.presentation=Emote, .condition=Losing, .repeat=true},
	};
	
	s_taunts[Creature::Doxy] =
	{
		Taunt::Data{.text="flutters its dark wings",
			.presentation=Emote, .repeat=true},
		Taunt::Data{.text="flies a loop",
			.presentation=Emote, .repeat=true},
		Taunt::Data{.text="buzzes excitedly",
			.presentation=Emote, .condition=Winning, .repeat=true},
		Taunt::Data{.text="grins maliciously",
			.presentation=Emote, .condition=Winning, .repeat=true},
		Taunt::Data{.text="bares its pointy teeth",
			.presentation=Emote, .condition=Losing, .repeat=true},
	};

	s_taunts[Creature::Imp] =
	{
		Taunt::Data{.text="flexes its claws",
			.presentation=Emote, .rarity=2, .repeat=true},
		Taunt::Data{.text="narrows its beady eyes",
			.presentation=Emote, .rarity=3, .repeat=true},
		Taunt::Data{.text="looks around furtively",
			.presentation=Emote, .rarity=3, .repeat=true},
		Taunt::Data{.text="produces a rasping snarl",
			.presentation=Emote, .rarity=3, .repeat=true},
		Taunt::Data{.text="makes an unpleasant noise in its throat",
			.presentation=Emote, .rarity=2, .repeat=true},
		Taunt::Data{.text="gives you a devilish smile",
			.presentation=Emote, .condition=Winning, .rarity=3, .repeat=true},
		Taunt::Data{.text="swallows a bug",
			.presentation=Emote, .rarity=6},
		Taunt::Data{.text="hisses angrily",
			.presentation=Emote, .condition=Losing, .repeat=true},
		Taunt::Data{.text="winks sadistically",
			.presentation=Emote, .condition=Winning},
		Taunt::Data{.text="licks its lips",
			.presentation=Emote, .condition=Winning, .repeat=true},
		Taunt::Data{.text="rattles with delight",
			.presentation=Emote, .condition=PlayerStatus, .subtype=Status::Prone, .repeat=true},
		Taunt::Data{.text="waves its hands in celebration",
			.presentation=Emote, .condition=PlayerStatus, .subtype=Status::Prone},
	};

	s_taunts[Creature::HarryTheHufflepuff_1] =
	{
		Taunt::Data{.text="This is too much work."},
		Taunt::Data{.text="Do we need to do this?"},
		Taunt::Data{.text="Do you know anything about house-elves?"},
		Taunt::Data{.text="Robes are great.  No need for trousers.",
			.rarity=3},
		Taunt::Data{.text="Could you make this a little easier?",
			.condition=Losing},
		Taunt::Data{.text="This is more effort than it's worth.",
			.condition=Losing},
		Taunt::Data{.text="Being bullied is easier than resisting.",
			.condition=Losing},
		Taunt::Data{.text="You're waving your wand too much.",
			.condition=PlayerMiscast},
		Taunt::Data{.text="Try doing it more slowly.",
			.condition=PlayerMiscast},
		Taunt::Data{.text="Time for my famous Harry-Kari strategy.",
			.condition=AttackSpell},
	};

	// TODO: Add a way for Fred and George to finish one other's sentences.
	s_taunts[Creature::Fred_Shop] =
	{
		Taunt::Data{.text="Psst!  Over here!",
			.condition=Greeting, .repeat=true},

		Taunt::Data{.text="Have you collected any beans for us?",
			.condition=ShopAttract},
		Taunt::Data{.text="Anything you need?",
			.condition=ShopAttract},

		Taunt::Data{.text="A pleasure doing business.",
			.condition=ShopDeal, .repeat=true},
		Taunt::Data{.text="Discerning, as always.",
			.condition=ShopDeal, .rarity=3},

		Taunt::Data{.text="Nothing catch your fancy?",
			.condition=ShopNoDeal, .repeat=true},
		Taunt::Data{.text="Next time, then.",
			.condition=ShopNoDeal, .repeat=true},
		Taunt::Data{.text="Tough customer!",
			.condition=ShopNoDeal, .rarity=3, .repeat=true},

		Taunt::Data{.text="Don't give up!",
			.condition=ShopLeaving},
		Taunt::Data{.text="Give 'em our regards!",
			.condition=ShopLeaving},

		// TODO - Add some lines for combat mode.
	};

	s_taunts[Creature::George_Shop] =
	{
		Taunt::Data{.text="Care to trade?",
			.condition=ShopAttract},
		Taunt::Data{.text="Are you running short of beans?",
			.condition=ShopAttract},
		Taunt::Data{.text="Anything you'd like to sell?",
			.condition=ShopAttract},

		Taunt::Data{.text="A pleasure doing business.",
			.condition=ShopDeal, .repeat=true},
		Taunt::Data{.text="I hope they're tasty!",
			.condition=ShopDeal, .rarity=3},
		Taunt::Data{.text="Enjoy the beans!",
			.condition=ShopDeal, .rarity=3},

		Taunt::Data{.text="We're on your side, you know.",
			.condition=ShopLeaving},
		Taunt::Data{.text="We're huge fans of your work.",
			.condition=ShopLeaving},
		Taunt::Data{.text="Loved that bit with the Sorting Hat.",
			.condition=ShopLeaving, .rarity=12},
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
			Draw::add_message(std::format("{} says, \"{}\"", Grammar::You(taunter), message),
				taunter.colour());
		}
		else if (taunt.presentation == Presentation::Emote)
		{
			Draw::add_message(std::format("{} {}.", Grammar::You(taunter), message),
				taunter.colour());
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
