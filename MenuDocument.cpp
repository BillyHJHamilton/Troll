#include "MenuDocument.h"

#include "BearLibTerminal.h"

void MenuDocument::init(std::string&& content)
{
	m_text = content;
	m_on_complete = nullptr;
}

void MenuDocument::init(std::string&& content, VoidFunction on_complete)
{
	m_text = content;
	m_on_complete = on_complete;
}

void MenuDocument::draw_screen ()
{
	terminal_font("");
	terminal_print(0, 0, m_text.c_str());
}

void MenuDocument::handle_input (int key)
{
	switch(key)
	{
		case TK_ENTER:
		case TK_ESCAPE:
			if (m_on_complete)
			{
				m_on_complete();
			}
			else
			{
				Menu::back();
			}
			break;
	}
}
