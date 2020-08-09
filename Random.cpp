#include "Random.h"

#include <cassert>
#include <random>

std::mt19937 generator;

std::mt19937 & get_generator()
{
	return generator;
}

void init_random ()
{
	// use the heavy-duty (cryptographically secure) generator to seed the
	// Mersenne Twister generator (faster and adequate for games). 
	std::random_device r;
	generator = std::mt19937(r());
}

float random (float min, float max)
{
	std::uniform_real_distribution<float> dist(min, max);
	return dist(generator);
}

int random (int min, int max)
{
	std::uniform_int_distribution<int> dist(min, max);
	return dist(generator);
}

int random_index (int size)
{
	std::uniform_int_distribution<int> dist(0, size-1);
	return dist(generator);
}

bool coinflip ()
{
	static std::uniform_int_distribution<int> dist(0, 1);
	return dist(generator) == 1;
}

bool one_in (int x)
{
	assert(x >= 1);
	std::uniform_int_distribution<int> dist(1, x);
	return dist(generator) == 1;
}



