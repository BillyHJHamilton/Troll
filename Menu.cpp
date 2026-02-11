#include "Menu.h"

#include "BearLibTerminal.h"
#include <iomanip>
#include <string>
#include <functional>

#include "Creature.h"
#include "Debug.h"
#include "Draw.h"
#include "Game.h"
#include "House.h"
#include "Input.h"
#include "MapUtil.h"
#include "Player.h"
#include "Spell.h"
#include "VectorUtil.h"

namespace Menu
{

//-------------------------------------------------------------------------------------------------
// Dynamic Data

Type s_current_menu_type = Type::None;

using VoidFunction = std::function<void()>;
std::unordered_map<int,VoidFunction> s_input_map;

//VoidFunction s_custom_draw = nullptr;

// For document menus.
std::string s_document_content;

// For list menus.
struct ListOption
{
	std::string text;
	int value = 0;
};
std::vector<ListOption> s_option_list;
int s_selection = 0;

using ListDetailsFunc = std::function<void(ListOption)>;
ListDetailsFunc s_list_details_func = nullptr;

//-------------------------------------------------------------------------------------------------
// Static Data

const char* const c_doc_title =
	"\n"
	"  ------------------------------------\n\n"
	"  TTTTTTT RRRR    OOO    L      L     \n"
	"     T    R   R  O   O   L      L     \n"
	"     T    R  R  O     O  L      L     \n"
	"     T    RRR   O     O  L      L     \n"
	"     T    R  R   O   O   L      L     \n"
	"     T    R   R   OOO    LLLLLL LLLLLL\n\n"
	"  ------------------------------------\n"
	"      The Revenge Of Luna Lovegood    \n"
	"  ------------------------------------\n"
	"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
	"  (press enter)";

const char* const c_doc_help =
	"How To Play\n"
	"\n"
	"Movement:\n"
	"  To move, use the arrow keys or numpad.\n"
	"  If you have no numpad, use Home/End/PgUp/PgDn for diagonals.\n"
	"  For a long move, hold Ctrl and press the move key.\n"
	"\n"
	"Resting:\n"
	"  To skip a turn, press Space (or numpad 5).\n"
	"  To rest until fully healed, hold Ctrl and press Space (or numpad 5).\n"
	"\n"
	"Spellcasting:\n"
	"  To cast a spell, hold Shift and type the spell's two-letter abbreviation.\n"
	"  To see your spells, press ? (shift /).\n"
	"\n"
	"Spell Targeting:\n"
	"  Your current target is highlighted on the map.\n"
	"  To cycle between targets, press Tab.\n"
	"  To target a square manually, hold shift and use the move controls.\n"
	"\n"
	"To show these instructions again, press 'h'.\n";

//-------------------------------------------------------------------------------------------------
// Helper function declarations

void reset_list();
void open_menu(Type type);

void cursor_up();
void cursor_down();

void draw_document();
void draw_list();

void draw_selected_house(ListOption option);
void draw_selected_spell(ListOption option);

void select_house();

//-------------------------------------------------------------------------------------------------
// Public function implementations

void show_title()
{
	open_menu(Document);
	s_document_content = c_doc_title;
	s_input_map[TK_ENTER] = &show_house_selection;
	s_input_map[TK_ESCAPE] = &show_house_selection;
}

void show_help()
{
	open_menu(Document);
	s_document_content = c_doc_help;
}

void show_game_over()
{
	open_menu(Document);
	s_document_content = "Game Over.\n\nYou were defeated by ";
	s_document_content += Creature::long_name_from_type(Player::get_defeated_by());
	s_document_content += ".";

	s_input_map[TK_ENTER] = &Game::reset;
	s_input_map[TK_ESCAPE] = &Game::reset;
}

void show_house_selection()
{
	open_menu(List);
	reset_list();

	s_document_content = "What is your Hogwarts House?";
	s_list_details_func = draw_selected_house;

	for (int i = 0; i < House::Count; ++i)
	{
		s_option_list.push_back({House::name((House::Id)i), i});
	}

	s_input_map[TK_ENTER] = &select_house;
}

void show_spells_known()
{
	open_menu(List);
	reset_list();

	s_document_content = "Known spells:";
	s_list_details_func = draw_selected_spell;

	std::vector<Spell::Index> spells_known = Player::handle().spells_known();
	s_option_list.reserve(spells_known.size());
	for (Spell::Index spell_index : spells_known)
	{
		ListOption option;
		option.text = Spell::get_abbrev(spell_index) + " " + Spell::get_name(spell_index);
		option.value = (int)spell_index;
		s_option_list.push_back(option);
	}

	s_input_map[TK_ENTER] = &Menu::close;
	s_input_map[TK_ESCAPE] = &Menu::close;
}

void close()
{
	s_current_menu_type = None;
	Game::set_mode(GameMode::Normal);
	Input::clear();
}

void update_screen()
{
	switch (s_current_menu_type)
	{
		case Document:
			draw_document();
			break;
		case List:
			draw_list();
			break;
	}

	//if (s_custom_draw != nullptr)
	//{
	//	s_custom_draw();
	//}
}

void handle_input(int key)
{
	// Some aliases:
	if (key == TK_KP_8)
	{
		key = TK_UP;
	}
	else if (key == TK_KP_2)
	{
		key = TK_DOWN;
	}

	VoidFunction* func = Util::Find(s_input_map, key);
	if (func)
	{
		(*func)();
	}
}

//-------------------------------------------------------------------------------------------------
// Helper function implementations

void reset_list()
{
	s_document_content.clear();
	s_option_list.clear();
	s_selection = 0;
	s_list_details_func = nullptr;

	s_input_map[TK_UP] = &Menu::cursor_up;
	s_input_map[TK_DOWN] = &Menu::cursor_down;
}

void open_menu(Type type)
{
	s_current_menu_type = type;
	Game::set_mode(GameMode::Menu);

	s_input_map.clear();

	if (type == List)
	{
		reset_list();
	}
}

void cursor_up()
{
	if (s_selection > 0)
	{
		--s_selection;
	}
}

void cursor_down()
{
	if (s_selection < s_option_list.size() - 1)
	{
		++s_selection;
	}
}

void draw_document()
{
	terminal_font("");
	terminal_print(0, 0, s_document_content.c_str());
}

void draw_list()
{
	terminal_font("");
	dimensions_t dim = terminal_print(0, 0, s_document_content.c_str());
	int list_start = dim.height + 1;

	for (int i = 0; i < s_option_list.size(); ++i)
	{
		terminal_print(2, list_start + i, s_option_list[i].text.c_str());
	}

	// Show a cursor
	terminal_put(0, list_start + s_selection, '>');

	if (s_list_details_func &&
		Check(Util::IsValidIndex(s_option_list, s_selection)))
	{
		s_list_details_func(s_option_list[s_selection]);
	}
}

void draw_selected_house(ListOption option)
{
	terminal_font("");
	House::Id h = (House::Id)option.value;

	if (!House::is_valid(h))
	{
		return;
	}

	dimensions_t dim = terminal_print(40, 2, House::name(h));

	terminal_print_ext(40, 4, 60, 20, 1, House::description(h));
}

void draw_selected_spell(ListOption option)
{
	terminal_font("");

	Spell::Index s = (Spell::Index)s_option_list[s_selection].value;
	if (!Spell::is_valid_index(s))
	{
		return;
	}

	float const base_success = 100.0f - Spell::get_miscast_rate(s, Player::handle().skill_magic());
	int const damage = Spell::get_damage(s, Player::handle());
	int const range = Spell::get_range(s);
	Spell::TargetType target_type = Spell::get_target_type(s);

	std::stringstream ss;
	ss << Spell::get_name(s) << "\n\n";
	ss << std::setw(13) << std::left << "Difficulty:"
		<< std::setw(3) << std::left << Spell::get_difficulty(s)
		<< " (" << std::fixed << std::setprecision(0) << base_success << "%)\n";

	if (target_type == Spell::TargetType::Creature)
	{
		ss << std::setw(13) << std::left << "Accuracy:" << Spell::get_accuracy(s) << "\n";
	}

	if (range > 0)
	{
		ss << std::setw(13) << std::left << "Range:" << range << "\n";
	}

	if (damage > 0)
	{
		ss << std::setw(13) << std::left << "Damage:" << damage << "\n";
	}

	dimensions_t dim = terminal_print(40, 2, ss.str().c_str());
	terminal_print_ext(40, 2 + dim.height, 60, 20, 1, Spell::get_description(s));
}

void select_house()
{
	House::Id house = (House::Id)s_option_list[s_selection].value;
	assert(House::is_valid(house));
	Creature::reset_player_stats(house);
	Menu::close();
}

}
