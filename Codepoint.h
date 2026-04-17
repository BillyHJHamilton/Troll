#pragma once

// Named constants for useful unicode characters.
// We're omitting the customary c_ here since the entire namespace is constants.
namespace Codepoint
{
	int constexpr UppercasePsi = 0x3A8;

	int constexpr ArrowUp = 0x2191;
	int constexpr ArrowDown = 0x2193;
	int constexpr BackwardsSquiggle = 0x3e8;

	int constexpr BoxEmpty = 0x2610;
	int constexpr BoxCheck = 0x2611;
	int constexpr BoxX = 0x2612;

	int constexpr HandRight = 0x261b;
	int constexpr House = 0x2302;
	int constexpr SmallSquare = 0x25ab;

	int constexpr ShortBlock = 0x2583;
	int constexpr SolidBlock = 0x2588;

	int constexpr MidTilde = 0x2053; // lower than the default one
	
	// Custom glyphs.
	// These ones are in decimal for simplicity.
	int constexpr OpenCursor = 12337; // 0x3031
	int constexpr Chest = 12338;
	int constexpr FlipendoButton = 12339;
	int constexpr Portrait = 12340;
	int constexpr CaretDown = 12341;
	int constexpr CaretUp = 12342;
	int constexpr TorchUnlit = 12343;
	int constexpr TorchLit = 12344;
}
