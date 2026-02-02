#include "Creature.h"

namespace Creature
{

void init_gingerbread()
{
	char const * cstr_gryffindor = "red";
	char const * cstr_hufflepuff = "yellow";

	mix_gingerbread(Creature::Player, Identity::Player,
		/*Difficulty*/ 0.0f, /*Probability*/ 0.0f,
		"You", "You", '@', "white", Gender::Female,
		/*Magic*/ 10, /*HP*/ 10, "VM FP TA LM");

	mix_gingerbread(Creature::Neville_0, Identity::NevilleLongbottom,
		/*Difficulty*/ 0.5f, /*Probability*/ 1.0f,
		"Neville", "Neville Longbottom", 'N', cstr_gryffindor, Gender::Male,
		/*Magic*/ 0, /*HP*/ 7, "VM TA");

	mix_gingerbread(Creature::ColinCreevy_0, Identity::ColinCreevy,
		/*Difficulty*/ 0.3f, /*Probability*/ 1.0f,
		"Colin", "Colin Creevy", 'C', cstr_gryffindor, Gender::Male,
		/*Magic*/ 8, /*HP*/ 5, "VM LM");
	
	mix_gingerbread(Creature::SallyAnne_0, Identity::SallyAnnePerks,
		/*Difficulty*/ 0.0f, /*Probability*/ 0.2f,
		"Sally-Anne", "Sally-Anne Perks", 'S', cstr_hufflepuff, Gender::Female,
		/*Magic*/ 0, /*HP*/ 3, "VM");

	// Generic student for testing purposes
	mix_gingerbread(Creature::Hufflepuff_1, Identity::Generic,
		/*Difficulty*/ 1.0f, /*Probability*/ 0.3f,
		"Hufflepuff", "First-Year Hufflepuff", 'H', cstr_hufflepuff, Gender::Male,
		/*Magic*/ 10, /*HP*/ 10, "VM FP TA LM");
}

} // namespace Creature
