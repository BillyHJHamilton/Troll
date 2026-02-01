#include "Creature.h"

namespace Creature
{

void init_gingerbread()
{
	mix_gingerbread(Creature::Player, Identity::Player,
		/*Difficulty*/ 0.0f, /*Probability*/ 0.0f,
		"You", "You", '@', "white", Gender::Female,
		/*Magic*/ 10, /*HP*/ 10, "VM FP TA LM");

	mix_gingerbread(Creature::Neville_0, Identity::Neville,
		/*Difficulty*/ 0.0f, /*Probability*/ 1.0f,
		"Neville", "Neville Longbottom", 'N', "red", Gender::Male,
		/*Magic*/ 0, /*HP*/ 7, "VM TA");

	mix_gingerbread(Creature::ColinCreevy_0, Identity::ColinCreevy,
		/*Difficulty*/ 0.0f, /*Probability*/ 1.0f,
		"Colin", "Colin Creevy", 'C', "red", Gender::Male,
		/*Magic*/ 8, /*HP*/ 5, "VM LM");

	// Generic student for testing purposes
	mix_gingerbread(Creature::Hufflepuff_1, Identity::Generic,
		/*Difficulty*/ 1.0f, /*Probability*/ 0.3f,
		"Hufflepuff", "First-Year Hufflepuff", 'H', "yellow", Gender::Male,
		/*Magic*/ 10, /*HP*/ 10, "VM FP TA LM");
}

} // namespace Creature
