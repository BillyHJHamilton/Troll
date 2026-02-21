#include "Debug.h"
#include "Scratch.h"
#include <cstddef>
#include <format>
#include <iostream>

namespace Scratchpad
{
	// We'll see how much we need...
	int constexpr c_ScratchpadSize = 4 * 1024 * 1024; // 4 MB
	char s_scratchpad [c_ScratchpadSize] alignas(std::max_align_t);

	int s_mark = 0;

	// Highest memory ever used - for debug purposes.
	int s_highwater = 0;

	// A true scratchpad would ignore free, but I want some validation that I haven't
	// accidentally std::move'd the scratched vector to somewhere permanent.
	int s_unfreed_allocations = 0;
	int s_unfreed_memory = 0;

	void* alloc(int bytes, int alignment)
	{
		// Align to next valid position
		int const extra = s_mark % alignment;
		int const padding = (extra) ? alignment - extra : 0;
		s_mark += padding;

		assert(s_mark + bytes < c_ScratchpadSize);

		void* mem = (void*)(&s_scratchpad[s_mark]);
		s_mark += bytes;
		s_highwater = std::max(s_highwater, s_mark);

		s_unfreed_memory += bytes;
		++s_unfreed_allocations;

		if (Debug::enabled(Debug::Memory))
		{
			std::cout << std::format(
				"Scratch alloc {} bytes with alignment {}, padding {}.  Got: {:x}\n",
				bytes, alignment, padding, (size_t)mem);
		}

		return mem;
	}

	void free(void* p, int bytes)
	{
		if (Debug::enabled(Debug::Memory))
		{
			std::cout << std::format("Scratch free {} bytes.\n", bytes);
		}

		assert(p >= (void*)&s_scratchpad
			&& p < (void*)(&s_scratchpad + c_ScratchpadSize));
		s_unfreed_memory -= bytes;
		--s_unfreed_allocations;

		if (s_unfreed_allocations == 0)
		{
			assert(s_unfreed_memory == 0);
			if (Debug::enabled(Debug::Memory))
			{
				std::cout << std::format("Scratch released with mark={}, highwater={}.\n",
					s_mark, s_highwater);
			}
			s_mark = 0;
		}
	}
}

