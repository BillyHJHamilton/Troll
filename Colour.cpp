#include "Colour.h"

namespace Colour
{

char const* rainbow(int x)
{
	switch(x % 21)
	{
		case 0:		return cstr_Red;
		case 1:		return cstr_Flame;
		case 2:		return cstr_Orange;
		case 3:		return cstr_Amber;
		case 4:		return cstr_Yellow;
		case 5:		return cstr_Lime;
		case 6:		return cstr_Chartreuse;
		case 7:		return cstr_Green;
		case 8:		return cstr_Sea;
		case 9:		return cstr_Turquoise;
		case 10:	return cstr_Cyan;
		case 11:	return cstr_Sky;
		case 12:	return cstr_Azure;
		// Use Light for these ones since it's a bit too dark otherwise.
		case 13:	return cstr_LightBlue;
		case 14:	return cstr_LightHan;
		case 15:	return cstr_LightViolet;
		case 16:	return cstr_LightPurple;
		// (End of light range.)
		case 17:	return cstr_Fuchsia;
		case 18:	return cstr_Magenta;
		case 19:	return cstr_Pink;
		default:	return cstr_Crimson;
	}
}

} // namespace Colour
