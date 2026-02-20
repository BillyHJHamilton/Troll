#pragma once

// Named constants for useful unicode characters.
// We're omitting the customary c_ here since the entire namespace is constants.
namespace Codepoint
{
	int constexpr ArrowUp = 0x2191;
	int constexpr ArrowDown = 0x2193;
	int constexpr BackwardsSquiggle = 0x3e8;

	int constexpr BoxEmpty = 0x2610;
	int constexpr BoxCheck = 0x2611;
	int constexpr BoxX = 0x2612;

	int constexpr CaratUp = 0x2c4;  // 2ef
	int constexpr CaratDown = 0x2c5;  // 2f0
	int constexpr HandRight = 0x261b;
	int constexpr House = 0x2302;
	int constexpr SmallSquare = 0x25ab;

	int constexpr ShortBlock = 0x2583;
	int constexpr SolidBlock = 0x2588;
}
