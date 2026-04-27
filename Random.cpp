#include "Random.h"

#include <cassert>
#include <random>
#include <iostream>

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

int index (int size)
{
	std::uniform_int_distribution<int> dist(0, size-1);
	return dist(generator);
}

int weighted_index(const std::vector<int> &weights)
{
	assert(weights.size() > 0);
	int sum = 0;
	for (int w : weights)
	{
		assert(w >= 0);
		sum += w;
	}
	assert(sum > 0);

	const int roll = Random::in_range(0, sum-1);
	int x = roll;

	for (int i = 0; i < (int)weights.size(); i++)
	{
		assert(weights[i] >= 0);
		x -= weights[i];
		if (x < 0)
			return i;
	}
	std::cerr << "RandomNumbers - weighted_index (int) failed to resolve." << std::endl;
	std::cerr << "Contents of weight vector: ";
	for (int w : weights)
	{
		std::cerr << w;
	}
	std::cerr << "Roll: " << roll << ", Subtraction result: " << x << "\n";
	return 0;
}

int weighted_index(const std::vector<float> &weights)
{
	assert(weights.size() > 0);
	float sum = 0.0f;
	for (float w : weights)
	{
		assert(w >= 0.0f);
		sum += w;
	}
	assert(sum > 0.0f);

	const float roll = Random::in_range(0.0f, sum);

	float x = roll;
	for (int i = 0; i < (int)weights.size(); i++)
	{
		x -= weights[i];
		if (x <= 0.0f)
			return i;
	}
	std::cerr << "RandomNumbers - weighted_index (int) failed to resolve." << std::endl;
	std::cerr << "Contents of weight vector: ";
	for (float w : weights)
	{
		std::cerr << w;
	}
	std::cerr << "Roll: " << roll << ", Subtraction result: " << x << "\n";
	return 0;
}

// Copy-pasted from the above
int weighted_index(const IntTempList &weights)
{
	assert(weights.size() > 0);
	int sum = 0;
	for (int w : weights)
	{
		assert(w >= 0);
		sum += w;
	}
	assert(sum > 0);

	const int roll = Random::in_range(0, sum-1);
	int x = roll;

	for (int i = 0; i < (int)weights.size(); i++)
	{
		assert(weights[i] >= 0);
		x -= weights[i];
		if (x < 0)
			return i;
	}
	std::cerr << "RandomNumbers - weighted_index (int) failed to resolve." << std::endl;
	std::cerr << "Contents of weight vector: ";
	for (int w : weights)
	{
		std::cerr << w;
	}
	std::cerr << "Roll: " << roll << ", Subtraction result: " << x << "\n";
	return 0;
}

// Copy-pasted from the above since I didn't feel like making a template
// (since there are some small differences between the int and float versions).
int weighted_index(const FloatTempList &weights)
{
	assert(weights.size() > 0);
	float sum = 0.0f;
	for (float w : weights)
	{
		assert(w >= 0.0f);
		sum += w;
	}
	assert(sum > 0.0f);

	const float roll = Random::in_range(0.0f, sum);

	float x = roll;
	for (int i = 0; i < (int)weights.size(); i++)
	{
		x -= weights[i];
		if (x <= 0.0f)
			return i;
	}
	std::cerr << "RandomNumbers - weighted_index (int) failed to resolve." << std::endl;
	std::cerr << "Contents of weight vector: ";
	for (float w : weights)
	{
		std::cerr << w;
	}
	std::cerr << "Roll: " << roll << ", Subtraction result: " << x << "\n";
	return 0;
}

int weighted_index(int const weights[], int size)
{
	assert(size > 0);
	int sum = 0;
	for (int i = 0; i < size; ++i)
	{
		assert(weights[i] >= 0);
		sum += weights[i];
	}
	assert(sum > 0);

	int const roll = Random::in_range(0, sum - 1);
	int x = roll;

	for (int i = 0; i < size; ++i)
	{
		assert(weights[i] >= 0);
		x -= weights[i];
		if (x < 0)
			return i;
	}
	std::cerr << "RandomNumbers - weighted_index (int) failed to resolve." << std::endl;
	std::cerr << "Contents of weight array: ";
	for (int i = 0; i < size; ++i)
	{
		std::cerr << weights[i] << " ";
	}
	std::cerr << "Roll: " << roll << ", Subtraction result: " << x << "\n";
	return 0;
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

CompassDirection compass_direction(bool include_no_move)
{
	int const max_dir = include_no_move ? c_CompassNoMove : c_CompassSoutheast; 
	return (CompassDirection)in_range(c_CompassEast, max_dir);
}

}

