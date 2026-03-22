#include "MenuPrologue.h"

#include "Config.h"
#include "Debug.h"
#include "Random.h"
#include "Score.h"
#include "VectorUtil.h"

#include "BearLibTerminal.h"

//-----------------------------------------------------------------------------
// Prologue text

namespace Prologue
{
	int constexpr Count = 5;
	char const* FirstWeeks = "Your first few weeks at Hogwarts could have gone a lot better.  Truth be told, they could hardly have gone worse.\n\nIt all started when you set fire to the Sorting Hat.  Well, that's not entirely true.  Before that, there was the incident with the boats and the giant squid.  And of course, your small accident on the Hogwarts Express.  Come to think of it, your visit to Diagon Alley didn't go so swimmingly either.\n\nBut ever since the Sorting Hat, things had certainly taken a turn for the worse...";
	char const* FinalExams = "Professor Dumbledore rose to his feet, and a hush settled over the Great Hall.\n\n\"I'm sure many of you have already heard the rumours,\" he began.  \"This year, we're going to do something a little bit different for our final exams...\"";
	char const* Outlaw = "Humiliated - framed - expelled!\n\nBefore today, you never thought of yourself as an outlaw.  Sure, you might have broken a few school rules, for the right reasons and with the best of intentions, for the most part.  You've done your share of underage magic, and suffered through a few detentions.  Still, up to today, you've done your best to stay on the side of the angels.  You've flirted with trouble, but never gone right up and asked it to a date in Hogsmeade.\n\nBut now?  After this?\n\nYou're not giving up your wand without a fight.";
	char const* IceSlide = "The spell challenges were getting a little out of hand.\n\nThere was no denying that the students liked them.  And indeed, there were certain pedagogical advantages to obstacle courses over traditional classroom instruction.  A chance to practice one's spellwork while climbing up ledges and searching for secret doors.  It was a little closer to real life, wasn't it?  No harm in that.\n\nBut still, things were getting out of hand.  Quirrell had been the first to introduce bottomless pits.  Then Snape had added that collapsing bridge, and Sprout had set loose live doxies in the greenhouse.  It was becoming something of an arms race.  Not to mention that the challenges were getting longer.  By now, half the castle seemed to be filled with sliding block puzzles, giant pendulums, and weight-activated fire crab traps.  Though, strangely enough, they never seemed to run out of vacant rooms.\n\nProfessor Flitwick waved his wand, putting the final touches on his latest giant ice slide.  No, things were definitely getting out of hand.";
	char const* Twilight = "About three things I was absolutely positive.\n\nFirst, Severus Snape was a vampire.\n\nSecond, there was a part of him - and I didn’t know how potent that part might be - that thirsted for my blood.\n\nAnd third, I was unconditionally and irrevocably in love with him.";
}

//-----------------------------------------------------------------------------
// MenuPrologue

void MenuPrologue::refresh()
{
	int const best_score = (Score::num_scores() > 0) ?
		Score::read_entry(0).points :
		0;

	std::vector<char const*, Scratch<char const*>> options;
	options.reserve(Prologue::Count);

	options.push_back(Prologue::FirstWeeks);

	if (best_score >= 200)
	{
		options.push_back(Prologue::FinalExams);
		options.push_back(Prologue::Outlaw);
	}

	if (best_score >= 600 && Random::one_in(3))
	{
		options.push_back(Prologue::IceSlide);
	}

	if (best_score >= 2000 && Random::one_in(5))
	{
		options.push_back(Prologue::Twilight);
	}

	assert(Util::Size(options) <= Prologue::Count);

	m_text = Random::from_vector(options);
}

void MenuPrologue::draw_screen ()
{
	terminal_font("");

	if (Check(m_text != nullptr))
	{
		int constexpr c_Width = 70;
		int const c_Height = Config::get_height() - 3;
		int const c_Left = (Config::get_width() - c_Width) / 2;

		terminal_print_ext(c_Left, 0, c_Width, c_Height, TK_ALIGN_MIDDLE, m_text);
	}
}

Input::Result MenuPrologue::handle_input (int key)
{
	switch(key)
	{
#if _DEBUG
		case TK_R:
			refresh();
			return Input::Result::Handled;
#endif
		case TK_ESCAPE:
		case TK_ENTER:
			Menu::close();
			return Input::Result::Handled;
	}

	return Input::Result::Skipped;
}
