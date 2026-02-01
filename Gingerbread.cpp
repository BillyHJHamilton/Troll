#include "Creature.h"

namespace Creature
{

void init_gingerbread()
{
	mix_gingerbread(
		Creature::Player, Identity::Player, -1.0f, 0.0f,
		"You", "You", '@', "white", Gender::Female,
		/*Magic*/ 10, /*HP*/ 10, "VM FP TA LM");

	mix_gingerbread(
		Creature::Neville_0, Identity::Neville, 0.0f, 1.0f,
		"Neville", "Neville Longbottom", 'N', "red", Gender::Male,
		/*Magic*/ 0, /*HP*/ 7, "VM TA");

	mix_gingerbread(Creature::ColinCreevy_0, Identity::ColinCreevy, 0.0f, 1.0f,
		"Colin", "Colin Creevy", 'C', "red", Gender::Male,
		/*Magic*/ 8, /*HP*/ 5, "VM LM");
}

} // namespace Creature
