#include "Random.h"

#include <cassert>
#include <random>

namespace Random
{

std::mt19937 generator;

std::mt19937 & get_generator()
{
	return generator;
}

void init ()
{
	// use the heavy-duty (cryptographically secure) generator to seed the
	// Mersenne Twister generator (faster and adequate for games). 
	std::random_device r;
	generator = std::mt19937(r());
}

float in_range (float min, float max)
{
	std::uniform_real_distribution<float> dist(min, max);
	return dist(generator);
}

int in_range (int min, int max)
{
	std::uniform_int_distribution<int> dist(min, max);
	return dist(generator);
}

Vec2 in_box(Box2 b)
{
	return
	{
		Random::in_range(b.min.x, b.inner_max().x),
		Random::in_range(b.min.y, b.inner_max().y)
	};
}

std::size_t index (std::size_t size)
{
	std::uniform_int_distribution<std::size_t> dist(0, size-1);
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

}

