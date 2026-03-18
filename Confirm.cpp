#include "Confirm.h"
#include "Draw.h"
#include "Game.h"

#include "BearLibTerminal.h"

namespace Confirm
{

// NOTE: Currently this system only works at the end of a turn.
// If we wanted to confirm in the *middle* of a turn, we'd need to do something more drastic,
// like redraw the screen, then trap execution in a mini input loop, which only handles basic
// inputs (like quit game, resize window) and the required confirmations.

//-------------------------------------------------------------------------------------------------
// Data

enum class Mode : int
{
	None = c_Invalid,
	PressEnter,
};
static Mode s_confirm_mode = Mode::None;

static std::function<void(void)> s_on_enter;

//-------------------------------------------------------------------------------------------------
// Interface

void clear()
{
	s_confirm_mode = Mode::None;
	s_on_enter = nullptr;
}

Input::Result handle_input(int key)
{
	if (s_confirm_mode == Mode::PressEnter)
	{
		if (key == TK_ENTER)
		{
			Game::set_mode(GameMode::Normal); // Call first since callback might overwrite it.
			if (s_on_enter != nullptr)
			{
				s_on_enter();
			}
			return Input::Result::Handled;
		}
	}

	return Input::Result::Skipped;
}

void press_enter(std::function<void(void)> on_enter)
{
	Draw::add_message("(Press enter)");
	s_on_enter = on_enter;
	s_confirm_mode = Mode::PressEnter;
	Game::set_mode(GameMode::Confirm);
}

} // namespace Confirm
