#pragma once

#include <cassert>
#include <climits>

namespace Scratchpad
{
	void* alloc(int bytes, int alignment);
	void free(void* p, int bytes);
	
	bool is_empty();
}

// Scratchpad allocator
// This allocator is intended to be used for temporary vectors, etc.
// It resets as soon as all current allocations are freed.
template<class T>
struct Scratch
{
	using value_type = T;

	Scratch<T>() noexcept {}

	template<class OtherType>
	Scratch<T>(const Scratch<OtherType>&) noexcept {}

	[[nodiscard]] value_type* allocate(size_t n)
	{
		size_t const bytes = n * sizeof(value_type);
		size_t constexpr alignment = alignof(value_type);
		assert(bytes < (size_t)INT_MAX);
		return (value_type*) Scratchpad::alloc((int)bytes, (int)alignment);
	}

	void deallocate(value_type* p, size_t n) noexcept
	{
		size_t const bytes = n * sizeof(value_type);
		assert(bytes < (size_t)INT_MAX);
		Scratchpad::free(p, (int)bytes);
	}
};

template<class T, class U>
bool operator==(const Scratch<T>&, Scratch<U>&) { return true; }

template<class T, class U>
bool operator!=(const Scratch<T>&, Scratch<U>&) { return false; }
