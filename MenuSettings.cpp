#include "MenuSettings.h"

#include "Config.h"

#include "BearLibTerminal.h"

//-------------------------------------------------------------------------------------------------
// Main debug menu

// TODO: It would be better to show an on/off for each setting, like in the logging menu

void MenuSettings::init()
{
	set_title("Game Settings:");
	set_options(
	{
		{"Back", SettingsOption::Back},
		{"Toggle Font Size", SettingsOption::ToggleFontSize},
	});
}

void MenuSettings::handle_input (int key)
{
	if (key == TK_ENTER)
	{
		switch(get_selected().value)
		{
			case SettingsOption::Back:
				Config::save();
				Menu::back();
				break;
			case SettingsOption::ToggleFontSize:
				Config::toggle_font_size();
				break;
		}
	}
	else
	{
		MenuList::handle_input(key);
	}
}

/*
void MenuDebugLogCategories::refresh()
{
	set_title("Log Categories:");

	m_options.resize(Debug::Category::Count + 2);
	for (int i = 0; i < Debug::Category::Count; ++i)
	{
		Debug::Category category = (Debug::Category)(i);
		bool const enabled = Debug::enabled(category);
		m_options[i].label = std::format("[[{}]] {}",
			enabled ? "ON" : "  ",
			Debug::category_name(category));
		m_options[i].value = (int)category;
	}
	m_options[Debug::Category::Count] = {"Enable All", c_EnableAll};
	m_options[Debug::Category::Count + 1] = {"Disable All", c_DisableAll};
}*/
