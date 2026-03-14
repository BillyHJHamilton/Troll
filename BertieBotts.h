#pragma once

#include <string>

namespace BertieBotts
{
	void init();

	int random_flavour();
	char const* get_name(int flavour);
	char const* get_colour(int flavour);
	std::string get_name_capitalized(int flavour);

	// Not currently in use
	//int random_heal_amount(int flavour);

	std::string eat_message(int flavour);
}
