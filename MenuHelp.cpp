#include "MenuHelp.h"

#include "Config.h"

#include "BearLibTerminal.h"

const char* const MenuHelp::cstr_Overview =
	"TROLL is a game set in the world of Harry Potter fanfiction.  "
		"As a young witch or wizard, your goal is (TODO).  "
		"Along the way, you must explore the world, collect items, learn spells, "
		"and face many opponents.\n"
	"\n"
	"This is a roguelike game.  This means:\n"
	"  - As you play, your progress is automatically saved.\n"
	"  - If you lose, your save file is deleted and you have to start over.\n"
	"  - The world is randomly generated each time you play.\n"
	"\n"
	"To navigate menus like this one:\n"
	"  Up/Down       Move cursor\n"
	"  Left/Right    Skip to top/bottom\n"
	"  Enter         Make selection (where applicable)\n"
	"  Esc           Cancel or return to previous menu\n";

const char* const MenuHelp::cstr_Movement =
	"The basic movement controls are:\n"
	"\n"
	"  Arrow keys    Move\n"
	"  Space         Skip turn\n"
	"  Ctrl Space    Rest until fully healed\n"
	"  Right Click   Move to the square clicked\n"
	"\n"
	"You can also move with the numpad.  This is very convenient for moving diagonally.  "
	"The numpad 5 key has the same effect as Space.\n"
	"\n"
	"If you have no numpad, you can move diagonally with the following keys:\n"
	"\n"
	"      Home   PgUp\n"
	"          \\ /\n"
	"          / \\\n"
	"       End   PgDn\n"
	"\n"
	"A note about mouse movement: "
	"When enemies are in sight, right click will move you only one square at a time.  "
	"If there are no enemies in sight, right click will move you all the way to the destination, "
	"or until an enemy is spotted.";

const char* const MenuHelp::cstr_CastingSpells =
	"To cast a spell, there are two steps:\n"
	"  1. First, select the target.\n"
	"  2. Then, hold Shift and type the spell's two-letter abbreviation.\n"
	"\n"
	"Your current target is highlighted on the map.  "
		"The game automatically selects the first target that comes into view.  "
		"To change targets:\n"
	"\n"
	"  Tab           Cycle between visible targets\n"
	"  Shift Tab     Cycle backwards\n"
	"  Ctrl Tab      Target yourself\n"
	"  Shift (Move)  Move the targeting cursor (instead of the character)\n"
	"  Left Click    Target where you click\n"
	"\n"
	"Spell abbreviations:  Each spell has a two-letter abbreviation, such as FP for Flipendo.  "
	"To see a list of your spells, press ? (Shift /).\n";

const char* const MenuHelp::cstr_Items =
	"To collect items, simply walk onto them.  "
	"You can view and use items from your inventory.\n"
	"\n"
	"  Enter         Open inventory\n"
	"  Ctrl Enter    Use most recently used item again\n";

const char* const MenuHelp::cstr_OtherCommands =
	"Here are some other miscellaneous game commands:\n"
	"\n"
	"  Esc           Show in-game menu\n"
	"  X             Examine contents of target square\n"
	"  Ctrl (Move)   Move in a line until terrain changes or enemy appears\n"
	"  Ctrl C        Collect nearest item\n"
	"  Ctrl D        Move into darkness (nearest unexplored square)\n"
//	"  Ctrl E        Explore - until enemy appears or level is clear\n"
	"  Ctrl G        Go to targeted location\n";

void MenuHelp::draw_screen ()
{
	MenuList::draw_screen();

	Page page = (Page)get_selected().value;

	const char* page_text = nullptr;
	switch (page)
	{
		case Overview:
			page_text = cstr_Overview;
			break;
		case Movement:
			page_text = cstr_Movement;
			break;
		case CastingSpells:
			page_text = cstr_CastingSpells;
			break;
		case Items:
			page_text = cstr_Items;
			break;
		case OtherCommands:
			page_text = cstr_OtherCommands;
			break;
	}

	if (page_text)
	{
		//int const width = Config::get_width() - c_MenuWidth;
		int const width = 75;
		int const x0 = c_MenuWidth;
		dimensions_t dim = terminal_print(x0, 2, get_selected().label.c_str());
		int const height = Config::get_height() - (3 + dim.height);
		terminal_print_ext(x0, 3 + dim.height, width, height, 1, page_text);
	}
}

void MenuHelp::init()
{
	m_title = "How To Play:";
	set_options({
		{"Overview", Page::Overview},
		{"Movement", Page::Movement},  //   and Resting
		{"Casting Spells", Page::CastingSpells},
		{"Items", Page::Items},
		{"Other Commands", Page::OtherCommands},
	});
}
