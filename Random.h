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
	std::size_t index (std::size_t size);
	bool coinflip ();
	bool one_in (int x);

	template<class T>
	std::size_t index (std::vector<T> v)
	{
		return random_index(v.size());
	}

	template<class T>
	T from_vector (std::vector<T> v)
	{
		return v.at(Random::index(v.size()));
	}

	template<class T>
	void shuffle_vector (std::vector<T> v)
	{
		std::shuffle(v.begin(), v.end(), get_generator());
	}
}