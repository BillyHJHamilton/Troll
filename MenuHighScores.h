#pragma once

#include "Types.h"

#include "Menu.h"

#include <string>

class MenuHighScores : public IMenu
{
public:
	void show_game_over();
	void show_scores();

	virtual void draw_screen ();
	virtual Input::Result handle_input (int key);

protected:
	enum class Mode : byte
	{
		GameOver,
		DisplayOnly
	};
	Mode m_mode = Mode::DisplayOnly;

	std::string m_text;
};
