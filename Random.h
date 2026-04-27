#pragma once

#include "Geometry.h"
#include "VectorUtil.h"

#include<algorithm>
#include<random>

namespace Random
{
	void init ();
	std::mt19937& get_generator ();

	int in_range (int min, int max);
	float in_range (float min, float max);
	Vec2 in_box(Box2 b);
	int index (int size);
	int weighted_index(const std::vector<int> &weights);
	int weighted_index(const std::vector<float> &weights);
	int weighted_index(const IntTempList &weights);
	int weighted_index(const FloatTempList &weights);
	int weighted_index(int const weights[], int size);
	bool coinflip ();
	bool one_in (int x);
	CompassDirection compass_direction(bool include_no_move);

	template<class T, typename Alc>
	int index (std::vector<T,Alc> v)
	{
		return Random::index((int)v.size());
	}

	template<class T, typename Alc>
	T from_vector (std::vector<T,Alc> v)
	{
		return v.at(Random::index((int)v.size()));
	}

	template<typename T, typename Alc>
	void shuffle_vector (std::vector<T,Alc>& v)
	{
		std::shuffle(v.begin(), v.end(), get_generator());
	}
}