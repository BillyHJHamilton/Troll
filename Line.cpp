#include "Line.h"

#include "Debug.h"
#include "FloatGeometry.h"
#include "Grid.h"
#include "PerfTimer.h"
#include "VectorUtil.h"

#include <cassert>
#include <iostream>
#include <format>

namespace LineCache
{
// We will draw lines through each point in a square.
// The lines will all be extended to length 8 for simplicity/consistency.
//
// We should try not to store redundant lines.
// In some cases the lines through two points are the same (once extended to maximum).
// To detect this, we should start with the inner squares and trace outwards.
// I guess the lines targeted on inner squares can also be added onto outer squares, but not vice versa.
// 
// The sides of the square will have length (2*range + 1).
int constexpr c_MaxRange = 8;
int constexpr c_LineGridDimension = (2 * c_MaxRange) + 1;
int constexpr c_LineGridArea = c_LineGridDimension * c_LineGridDimension;

// Cache of precomputed lines.
Ragged<Vec2> s_line_cache;

// For each square in a grid around the origin, a list of (1 or 2) cached lines that target it.
// - Outer array index is the line's lookup index (see get_lookup_index below).
// - Inner array is the list of lines which target that point.
// Note: We don't include any entries at the origin, since EVERY line starts there.
// Note: Does not include every possible line passing through a square,
//       because this causes asymmetric LOS.
Ragged<int> s_line_lookup;

// However, we may include asymmetric lines to certain squares.
// These are only valid when tracing a line to a solid wall square.
std::vector<int> s_asymmetric_line_lookup;

//------------------------------------------------------------------------------
// Helper function declarations

int get_lookup_index(Vec2 pos);

void add_ring_of_lines_to_cache(int range);
void add_perfect_line_to_cache(Vec2 end);
void add_line_to_cache_both_modes(Vec2 end);
void add_line_to_cache(Vec2 end, LineItr::RoundMode mode);
void add_asymmetric_line_to_cache(Vec2 end);

//------------------------------------------------------------------------------
// Interface implementation

void init()
{
	PerfTimer perf0("LineCache init");

	// Initialize lookup table with empty vectors.
	s_line_lookup.resize(c_LineGridArea);
	s_asymmetric_line_lookup.resize(c_LineGridArea, c_Invalid);

	s_line_cache.reserve(184); // empirical result with max_range=8

	// Add all the lines to the cache and lookup table.

	// Start with the perfect straight lines and diagonals.
	add_perfect_line_to_cache({ 0, c_MaxRange});
	add_perfect_line_to_cache({ 0,-c_MaxRange});
	add_perfect_line_to_cache({ c_MaxRange, 0});
	add_perfect_line_to_cache({-c_MaxRange, 0});
	add_perfect_line_to_cache({ c_MaxRange, c_MaxRange});
	add_perfect_line_to_cache({ c_MaxRange,-c_MaxRange});
	add_perfect_line_to_cache({-c_MaxRange, c_MaxRange});
	add_perfect_line_to_cache({-c_MaxRange,-c_MaxRange});

	// Then work outwards, starting with ring 2 (since the inner ring is all perfect).
	for (int range = 2; range <= c_MaxRange; ++range)
	{
		add_ring_of_lines_to_cache(range);
	}
		
	if (Debug::enabled(Debug::Line))
	{
		std::cout << std::format("Cached {} lines with a grid area of {}.\n\n",
			s_line_cache.size(), c_LineGridArea);

		for (int i = 0; i < s_line_lookup.size(); ++i)
		{
			const int n = (int)s_line_lookup[i].size();
			std::cout << n << " ";
			if (i % c_LineGridDimension == (c_LineGridDimension - 1))
			{
				std::cout << "\n";
			}
		}

		// Check if any identical lines somehow got in.
		int duplicate_pairs = 0;
		for (int i = 0; i < s_line_cache.size(); ++i)
		{
			for (int j = i + 1; j < s_line_cache.size(); ++j)
			{
				if (s_line_cache[i] == s_line_cache[j])
				{
					++duplicate_pairs;
				}
			}
		}

		std::cout << std::format("\nFound {} pairs of duplicate lines.\n",
			duplicate_pairs);
	}
}

std::vector<int> const& get_lines(Vec2 relative_pos)
{
	assert(s_line_lookup.size() == c_LineGridArea);

	int lookup = get_lookup_index(relative_pos);

	// If pos is invalid, return the empty cell at the centre of the grid.
	if (!Util::IsValidIndex(s_line_lookup, lookup))
	{
		lookup = get_lookup_index({0,0});
	}

	return s_line_lookup.at(lookup);
}

std::vector<int> const& get_lines(Vec2 start, Vec2 end)
{
	return get_lines(end - start);
}

int get_asymmetric_line(Vec2 relative_pos)
{
	int lookup = get_lookup_index(relative_pos);
	if (Util::IsValidIndex(s_line_lookup, lookup))
	{
		return s_asymmetric_line_lookup[lookup];
	}
	return c_Invalid;
}

int get_asymmetric_line(Vec2 start, Vec2 end)
{
	return get_asymmetric_line(end - start);
}

int get_num()
{
	return (int)s_line_cache.size();
}

Vec2 read_line(int line_id, int step)
{
	if (step == 0)
	{
		return {0,0};
	}

	if (Check(Util::IsValidIndex(s_line_cache, line_id)))
	{
		std::vector<Vec2>& data = s_line_cache.at(line_id);
		int const index = step - 1; // we don't store pos 0
		if (Check(Util::IsValidIndex(data, index)))
		{
			return data[index];
		}
	}

	return {0,0};
}

//------------------------------------------------------------------------------
// Helper function implementations

int get_lookup_index(Vec2 pos)
{
	if (pos.x < -c_MaxRange || pos.x > c_MaxRange ||
		pos.y < -c_MaxRange || pos.y > c_MaxRange)
	{
		return c_Invalid;
	}

	// Transform from range (-r,r) to (0, 2r)
	int const adjusted_x = pos.x + c_MaxRange;
	int const adjusted_y = pos.y + c_MaxRange;

	// Map onto a one-dimensional array.
	return adjusted_x + (c_LineGridDimension * adjusted_y);
}

void add_ring_of_lines_to_cache(int range)
{
	// Sweep in from the diagonals towards the straights.
	// But skip the perfect straights and diagonals, which were already added.
	for (int i = (range - 1); i > 0; --i)
	{
		add_line_to_cache_both_modes({ i,  range});
		add_line_to_cache_both_modes({-i,  range});
		add_line_to_cache_both_modes({ i, -range});
		add_line_to_cache_both_modes({-i, -range});
		add_line_to_cache_both_modes({ range,  i});
		add_line_to_cache_both_modes({ range, -i});
		add_line_to_cache_both_modes({-range,  i});
		add_line_to_cache_both_modes({-range, -i});
	}
}

void add_perfect_line_to_cache(Vec2 end)
{
	int const new_line_index = (int)s_line_cache.size();
	std::vector<Vec2> new_line;
	new_line.reserve(c_MaxRange);

	Vec2 start{ 0,0 };
	LineItr itr(start, end, LineItr::RoundUp);
	for (int step = 1; step <= c_MaxRange; ++step)
	{
		++itr; // Skip the origin since it's the same for all lines.

		new_line.push_back(*itr);

		int const lookup = get_lookup_index(*itr);
		s_line_lookup[lookup].push_back(new_line_index);
	}

	s_line_cache.emplace_back(std::move(new_line));
}

void add_line_to_cache_both_modes(Vec2 end)
{
	add_line_to_cache(end, LineItr::RoundUp);
	add_line_to_cache(end, LineItr::RoundDown);

	add_asymmetric_line_to_cache(end);
}

void add_line_to_cache(Vec2 end, LineItr::RoundMode mode)
{
	std::vector<Vec2> new_line;
	new_line.reserve(c_MaxRange);

	Vec2 start{ 0,0 };
	LineItr itr(start, end, mode);
	for (int step = 1; step <= c_MaxRange; ++step)
	{
		++itr; // Skip the origin since it's the same for all lines.

		new_line.push_back(*itr);
	}

	int const new_lookup = get_lookup_index(end);
	
	// Brute force check for a duplicate line.
	for (int line_id = 0; line_id < Util::Size(s_line_cache); ++line_id)
	{
		if (s_line_cache[line_id] == new_line)
		{
			Util::AddUnique(s_line_lookup[new_lookup], line_id);
			return;
		}
	}

	// Didn't find a duplicate.  Add this line to cache.
	s_line_cache.emplace_back(std::move(new_line));
	s_line_lookup[new_lookup].push_back(Util::LastIndex(s_line_cache));
}

void add_asymmetric_line_to_cache(Vec2 end)
{
	float constexpr c_OffsetTowardCentre = 0.499f;

	std::vector<Vec2> new_line;
	new_line.reserve(c_MaxRange);

	Axis const long_axis = get_long_axis(end);
	Axis const other_axis = get_other_axis(long_axis);

	FloatVec2 constexpr fv_start{0.0f, 0.0f};

	FloatVec2 fv_end = FloatVec2::from_int(end);
	fv_end[other_axis] -= (c_OffsetTowardCentre * Math::Sign(fv_end[other_axis]));

	FloatLineItr itr(fv_start, fv_end);
	for (int step = 1; step <= c_MaxRange; ++step)
	{
		++itr; // Skip the origin since it's the same for all lines.

		new_line.push_back(*itr);
	}

	int const new_lookup = get_lookup_index(end);

	// Brute force check for a duplicate line.
	for (int line_id = 0; line_id < Util::Size(s_line_cache); ++line_id)
	{
		if (s_line_cache[line_id] == new_line)
		{
			s_asymmetric_line_lookup[new_lookup] = line_id;
			return;
		}
	}

	// Didn't find a duplicate.  Add this line to cache.
	s_line_cache.emplace_back(std::move(new_line));
	s_asymmetric_line_lookup[new_lookup] = Util::LastIndex(s_line_cache);
}

//------------------------------------------------------------------------------
// Line Cache Iterator

Itr::Itr(Vec2 start_pos, int line_id) :
	start(start_pos),
	current(start_pos),
	id(line_id),
	step(0)
{
}

int Itr::steps_left() const
{
	return c_MaxRange - step;
}

void Itr::advance()
{
	++step;

	if (finished())
	{
		return;
	}

	// Cached lines don't include start position since it's always (0,0).
	int const new_index = step - 1;

	Vec2 const offset = s_line_cache.at(id).at(new_index);
	current = start + offset;
}

void Itr::advance_and_loop()
{
	if (step == c_MaxRange)
	{
		start = current;
		step = 0;
	}

	advance();
}

} // namespace LineCache


//-------------------------------------------------------------------------------------------------
// Line Drawing Algorithm
// 
// Sources:
// - https://stackoverflow.com/questions/10060046/drawing-lines-with-bresenhams-line-algorithm
// - https://github.com/denismr/SymmetricPCVT/ (public domain)
// See also: Thong, Tran. "A symmetric linear algorithm for line segment generation." Computers & Graphics 6.1 (1982): 15-17.

LineItr::LineItr(Vec2 const& start, Vec2 const& end, LineItr::RoundMode mode)
	: current(start)
{
	int const dx = end.x - start.x;
	int const dy = end.y - start.y;

	int const dist_x = std::abs(dx);
	int const dist_y = std::abs(dy);

	int const step_x = (dx > 0) ? 1 : -1;
	int const step_y = (dy > 0) ? 1 : -1;

	int dist_short;

	if (dist_x >= dist_y) // if it's more horizontal
	{
		dist_long = dist_x;
		dist_short = dist_y;

		step[0] = { step_x, 0 };      // step on one axis
		step[1] = { step_x, step_y }; // step on both axes
	}
	else // more vertical
	{
		dist_long = dist_y;
		dist_short = dist_x;

		step[0] = { 0, step_y };      // step on one axis
		step[1] = { step_x, step_y }; // step on both axes
	}

	d_offset[0] = dist_short;				// change in offset when taking step on one axis
	d_offset[1] = dist_short - dist_long;	// change in offset when taking step on both axes

	offset = dist_short; // Take the first step (so we don't have to do it at start of advance)
	steps_left = dist_long;

	switch(mode)
	{
		case RoundUp:
		{
			modifier = 0;
			break;
		}

		case RoundDown:
		{
			modifier = -1;
			break;
		}

		default:
		case Thong:
		{
			modifier = (step_y == -1) ? -1 : 0;
			break;
		}
	}
}

void LineItr::advance()
{
	// To track movement on the short axis, we accumulate a fraction where
	// the implicit denominator is the long axis.  Each step, we add the short
	// axis to the numerator ("offset").  For example, if the line is (3,4),
	// then we are accumulating 3/4 of a square per step.

	// If we have accumulated more than half a square, step on both axes.
	// Otherwise, step on only one axis.
	// We check with multiplication to avoid integer truncation errors.
	// We may add a -1 modifier (which effectively turns it into a >=).
	int const step_on_both = (offset * 2) > dist_long + modifier;

	current += step[step_on_both];
	offset += d_offset[step_on_both];

	--steps_left;
}
