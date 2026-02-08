#include "Creature.h"

namespace Creature
{

void init_gingerbread()
{
	char const * cstr_gryffindor = "light red";
	char const * cstr_hufflepuff = "light yellow";
	char const * cstr_slytherin = "light green";
	char const * cstr_ravenclaw = "light blue";

	mix_gingerbread(Creature::Player, Identity::Player,
		/*Difficulty*/ 0.0f, /*Probability*/ 0.0f,
		"You", "You", '@', "white", Gender::Female,
		/*Magic*/ 70, /*HP*/ 90, "VM FP TA LM MW RS LC FN SP IP BT FM");

	mix_gingerbread(Creature::Neville_0, Identity::NevilleLongbottom,
		/*Difficulty*/ 0.5f, /*Probability*/ 1.0f,
		"Neville", "Neville Longbottom", 'N', cstr_gryffindor, Gender::Male,
		/*Magic*/ 0, /*HP*/ 7, "VM FP");

	mix_gingerbread(Creature::ColinCreevy_0, Identity::ColinCreevy,
		/*Difficulty*/ 0.3f, /*Probability*/ 1.0f,
		"Colin", "Colin Creevy", 'C', cstr_gryffindor, Gender::Male,
		/*Magic*/ 8, /*HP*/ 5, "VM MW");
	
	mix_gingerbread(Creature::SallyAnne_0, Identity::SallyAnnePerks,
		/*Difficulty*/ 0.0f, /*Probability*/ 0.2f,
		"Sally-Anne", "Sally-Anne Perks", 'S', cstr_hufflepuff, Gender::Female,
		/*Magic*/ 0, /*HP*/ 3, "VM", "Faint.Disappear");

	mix_gingerbread(Creature::Harry_1, Identity::HarryPotter,
		/*Difficulty*/ 1.0f, /*Probability*/ 1.0f,
		"Harry", "Harry Potter", 'H', cstr_gryffindor, Gender::Male,
		/*Magic*/ 10, /*HP*/ 12, "VM FP TA");
	
	mix_gingerbread(Creature::Malfoy_1, Identity::DracoMalfoy,
		/*Difficulty*/ 1.0f, /*Probability*/ 1.0f,
		"Malfoy", "Draco Malfoy", 'M', cstr_slytherin, Gender::Male,
		/*Magic*/ 15, /*HP*/ 10, "VM FP LM");

	mix_gingerbread(Creature::Ron_2, Identity::RonWeasley,
		/*Difficulty*/ 2.0f, /*Probability*/ 1.0f,
		"Ron", "Ron Weasley", 'R', cstr_gryffindor, Gender::Male,
		/*Magic*/ 5, /*HP*/ 16, "FP VM"); // + FM

	mix_gingerbread(Creature::Hermione_2, Identity::HermioneGranger,
		/*Difficulty*/ 2.0f, /*Probability*/ 1.0f,
		"Hermione", "Hermione Granger", 'H', cstr_gryffindor, Gender::Female,
		/*Magic*/ 35, /*HP*/ 12, "VM MW LC"); // + FI

	mix_gingerbread(Creature::Crabbe_3, Identity::VincentCrabbe,
		/*Difficulty*/ 3.0f, /*Probability*/ 1.0f,
		"Crabbe", "Vincent Crabbe", 'C', cstr_slytherin, Gender::Male,
		/*Magic*/ 20, /*HP*/ 20, "FN RS");

	mix_gingerbread(Creature::Goyle_3, Identity::GregoryGoyle,
		/*Difficulty*/ 3.0f, /*Probability*/ 1.0f,
		"Goyle", "Gregory Goyle", 'G', cstr_slytherin, Gender::Male,
		/*Magic*/ 20, /*HP*/ 20, "VM FP TA");

	// Generic student for testing purposes
	mix_gingerbread(Creature::Hufflepuff_1, Identity::Generic,
		/*Difficulty*/ 0.6f, /*Probability*/ 0.3f,
		"Hufflepuff", "First-Year Hufflepuff", 'H', cstr_hufflepuff, Gender::Male,
		/*Magic*/ 6, /*HP*/ 4, "TA VM");//LM VM FP 
}

} // namespace Creature
