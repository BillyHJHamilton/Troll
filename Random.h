#pragma once

#pragma once
#include<vector>
#include<random>
#include<algorithm>

void init_random ();
std::mt19937& get_generator();

int random (int min, int max);
float random (float min, float max);
std::size_t random_index (std::size_t size); // randex, if you will
bool coinflip ();
bool one_in (int x);

template<class T>
std::size_t random_index (std::vector<T> v)
{
	return random_index(v.size());
}

template<class T>
T random_from_vector (std::vector<T> v)
{
	return v.at(random_index(v.size()));
}

template<class T>
void shuffle_vector (std::vector<T> v)
{
	std::shuffle(v.begin(), v.end(), get_generator());
}
