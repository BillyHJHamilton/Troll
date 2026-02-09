#pragma once

#include<vector>
#include<random>
#include<algorithm>
#include "Geometry.h"

namespace Random
{
	void init ();
	std::mt19937& get_generator ();

	int in_range (int min, int max);
	float in_range (float min, float max);
	Vec2 in_box(Box2 b);
	int index (int size);
	int weighted_index(const std::vector<float> &weights);
	bool coinflip ();
	bool one_in (int x);
	CompassDirection compass_direction(bool include_no_move);

	template<class T>
	int index (std::vector<T> v)
	{
		return Random::index((int)v.size());
	}

	template<class T>
	T from_vector (std::vector<T> v)
	{
		return v.at(Random::index((int)v.size()));
	}

	template<class T>
	void shuffle_vector (std::vector<T> v)
	{
		std::shuffle(v.begin(), v.end(), get_generator());
	}
}