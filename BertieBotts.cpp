#include "BertieBotts.h"
#include "Colour.h"
#include "Debug.h"
#include "Random.h"
#include "VectorUtil.h"

#include <cctype>
#include <format>

namespace BertieBotts
{

enum class Tastiness
{
	Wonderful,
	Good,
	Questionable,
	Terrible
};

struct BeanFlavour
{
	char const* colour;
	char const* name;
	Tastiness tastiness;
};

std::vector<BeanFlavour> s_flavours;
std::vector<int> s_weights;

void add_flavour(char const* colour, char const* name, int frequency, Tastiness tastiness)
{
	s_flavours.push_back({colour, name, tastiness});
	s_weights.push_back(frequency);
};

void init()
{
	int constexpr c_BeanReserveSize = 200;

	// This should only ever run once.
	assert(s_flavours.empty());
	assert(s_weights.empty());

	s_flavours.reserve(c_BeanReserveSize);
	s_weights.reserve(c_BeanReserveSize);

	int constexpr c_Common = 30;
	int constexpr c_Uncommon = 10;
	int constexpr c_Rare = 5;
	int constexpr c_VeryRare = 1;

	add_flavour(cstr_DarkWhite, "coconut", c_Common, Tastiness::Good);
	add_flavour(cstr_DarkWhite, "milk", c_Uncommon, Tastiness::Questionable);
	add_flavour(cstr_DarkWhite, "yoghurt", c_Rare, Tastiness::Good);
	add_flavour(cstr_DarkWhite, "salt", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_DarkWhite, "sugar", c_Rare, Tastiness::Good);
	add_flavour(cstr_DarkWhite, "cauliflower", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_DarkWhite, "lychee", c_VeryRare, Tastiness::Good);
	add_flavour(cstr_DarkWhite, "white blackberry", c_VeryRare, Tastiness::Questionable);
	add_flavour(cstr_DarkWhite, "marizpan", c_VeryRare, Tastiness::Wonderful);
	add_flavour(cstr_DarkWhite, "snow", c_VeryRare, Tastiness::Questionable);

	add_flavour(cstr_LighterGrey, "vanilla", c_Common, Tastiness::Wonderful);
	add_flavour(cstr_LighterGrey, "potato", c_Uncommon, Tastiness::Questionable);
	add_flavour(cstr_LighterGrey, "rice", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_LighterGrey, "envelope glue", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_LighterGrey, "dust bunny", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_LighterGrey, "soap", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_LighterGrey, "wool", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_LighterGrey, "sweat", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_LighterGrey, "slug", c_VeryRare, Tastiness::Terrible);

	add_flavour(cstr_Grey, "mushroom", c_Rare, Tastiness::Good);
	add_flavour(cstr_Grey, "sunflower seed", c_Rare, Tastiness::Good);
	add_flavour(cstr_Grey, "oyster", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_Grey, "cod liver oil", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_Grey, "lint", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_Grey, "tinfoil", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_Grey, "melted plastic", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_Grey, "haggis", c_VeryRare, Tastiness::Terrible);

	add_flavour(cstr_DarkerGrey, "liquorice", c_Common, Tastiness::Good);
	add_flavour(cstr_DarkerGrey, "sesame", c_Rare, Tastiness::Good);
	add_flavour(cstr_DarkerGrey, "pepper", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_DarkerGrey, "black olive", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_DarkerGrey, "charcoal", c_Rare, Tastiness::Terrible);
	add_flavour(cstr_DarkerGrey, "black pudding", c_VeryRare, Tastiness::Terrible);

	add_flavour(cstr_Red, "strawberry", c_Common, Tastiness::Wonderful);
	add_flavour(cstr_Red, "raspberry", c_Uncommon, Tastiness::Wonderful);
	add_flavour(cstr_Red, "peppermint", c_Uncommon, Tastiness::Good);
	add_flavour(cstr_Red, "apple", c_Uncommon, Tastiness::Good);
	add_flavour(cstr_Red, "red currant", c_VeryRare, Tastiness::Questionable);

	add_flavour(cstr_LightRed, "tomato", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_LightRed, "fig", c_Rare, Tastiness::Good);
	add_flavour(cstr_LightRed, "radish", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_LightRed, "watermelon", c_Rare, Tastiness::Wonderful);
	add_flavour(cstr_LightRed, "lobster", c_VeryRare, Tastiness::Questionable);
	add_flavour(cstr_LightRed, "cinderblock", c_VeryRare, Tastiness::Terrible);

	add_flavour(cstr_DarkRed, "paprika", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_DarkRed, "ketchup", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_DarkRed, "rose", c_VeryRare, Tastiness::Good);
	add_flavour(cstr_DarkRed, "dried blood", c_VeryRare, Tastiness::Terrible);

	add_flavour(cstr_Flame, "pizza", c_Rare, Tastiness::Good);
	add_flavour(cstr_Flame, "cream soda", c_Rare, Tastiness::Wonderful);
	add_flavour(cstr_Flame, "blood orange", c_VeryRare, Tastiness::Good);
	add_flavour(cstr_Flame, "saffron", c_VeryRare,	Tastiness::Good);
	add_flavour(cstr_Flame, "rusty metal", c_VeryRare, Tastiness::Terrible);

	add_flavour(cstr_LightFlame, "peach", c_Uncommon, Tastiness::Wonderful);

	add_flavour(cstr_DarkFlame, "cinnamon", c_Uncommon, Tastiness::Good);
	add_flavour(cstr_DarkFlame, "black tea", c_Rare, Tastiness::Good);
	add_flavour(cstr_DarkFlame, "chili powder", c_VeryRare, Tastiness::Questionable);
	add_flavour(cstr_DarkFlame, "leather", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_DarkFlame, "mud", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_DarkFlame, "plum pudding", c_VeryRare, Tastiness::Good);

	add_flavour(cstr_Orange, "orange", c_Common, Tastiness::Good);
	add_flavour(cstr_Orange, "carrot", c_Uncommon, Tastiness::Questionable);
	add_flavour(cstr_Orange, "marmalade", c_Uncommon, Tastiness::Good);
	add_flavour(cstr_Orange, "pumpkin", c_Uncommon, Tastiness::Questionable);
	add_flavour(cstr_Orange, "rutabaga", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_Orange, "papaya", c_Rare, Tastiness::Good);
	add_flavour(cstr_Orange, "apricot", c_Rare, Tastiness::Good);
	add_flavour(cstr_Orange, "sawdust", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_Orange, "persimmon", c_VeryRare, Tastiness::Good);
	add_flavour(cstr_Orange, "kumquat", c_VeryRare, Tastiness::Good);
	add_flavour(cstr_Orange, "orange watermelon", c_VeryRare, Tastiness::Good);

	add_flavour(cstr_LightOrange, "toffee", c_Uncommon, Tastiness::Good);
	add_flavour(cstr_LightOrange, "spaghetti", c_Rare, Tastiness::Good);
	add_flavour(cstr_LightOrange, "oatmeal", c_Rare, Tastiness::Good);
	add_flavour(cstr_LightOrange, "caramel", c_Rare, Tastiness::Wonderful);
	add_flavour(cstr_LightOrange, "canteloupe", c_Rare, Tastiness::Good);
	add_flavour(cstr_LightOrange, "peanut butter", c_Rare, Tastiness::Good);
	add_flavour(cstr_LightOrange, "toast", c_Rare, Tastiness::Good);
	add_flavour(cstr_LightOrange, "horseradish", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_LightOrange, "croissant", c_VeryRare, Tastiness::Good);
	add_flavour(cstr_LightOrange, "garbanzo bean", c_VeryRare, Tastiness::Questionable);
	add_flavour(cstr_LightOrange, "parmesan", c_VeryRare, Tastiness::Good);
	add_flavour(cstr_LightOrange, "earwax", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_LightOrange, "tripe", c_VeryRare, Tastiness::Terrible);

	add_flavour(cstr_DarkerOrange, "chocolate", c_Common, Tastiness::Wonderful);
	add_flavour(cstr_DarkerOrange, "coffee", c_Uncommon, Tastiness::Good);
	add_flavour(cstr_DarkerOrange, "lentil", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_DarkerOrange, "baked bean", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_DarkerOrange, "dirt", c_Rare, Tastiness::Terrible);
	add_flavour(cstr_DarkerOrange, "dog food", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_DarkerOrange, "haggis", c_VeryRare, Tastiness::Terrible);

	add_flavour(cstr_Amber, "honey", c_Rare, Tastiness::Wonderful);
	add_flavour(cstr_Amber, "mango", c_Rare, Tastiness::Wonderful);

	add_flavour(cstr_LightAmber, "almond", c_Rare, Tastiness::Good);
	add_flavour(cstr_LightAmber, "fried egg", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_LightAmber, "champagne", c_VeryRare, Tastiness::Good);

	add_flavour(cstr_Yellow, "lemon", c_Common, Tastiness::Good);
	add_flavour(cstr_Yellow, "pineapple", c_Uncommon, Tastiness::Good);
	add_flavour(cstr_Yellow, "sour lemon", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_Yellow, "lemonade", c_VeryRare, Tastiness::Wonderful);
	add_flavour(cstr_Yellow, "highlighter ink", c_VeryRare, Tastiness::Terrible);

	add_flavour(cstr_LightYellow, "banana", c_Common, Tastiness::Good);
	add_flavour(cstr_LightYellow, "popcorn", c_Uncommon, Tastiness::Good);
	add_flavour(cstr_LightYellow, "corn", c_Uncommon, Tastiness::Good);
	add_flavour(cstr_LightYellow, "butter", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_LightYellow, "cheese", c_Rare, Tastiness::Good);
	add_flavour(cstr_LightYellow, "custard", c_VeryRare, Tastiness::Wonderful);

	add_flavour(cstr_DarkYellow, "mustard", c_Uncommon, Tastiness::Questionable);
	add_flavour(cstr_DarkYellow, "passionfruit", c_Rare, Tastiness::Good);
	add_flavour(cstr_DarkYellow, "curry", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_DarkYellow, "old banana", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_DarkYellow, "sulphur", c_VeryRare, Tastiness::Terrible);

	add_flavour(cstr_Lime, "bogey", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_Lime, "chlorine", c_VeryRare, Tastiness::Terrible);

	add_flavour(cstr_LightLime, "green apple", c_Uncommon, Tastiness::Good);
	add_flavour(cstr_LightLime, "pistachio", c_VeryRare, Tastiness::Good);
	add_flavour(cstr_LightLime, "sprouts", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_LightLime, "green tea", c_Rare, Tastiness::Good);
	add_flavour(cstr_LightLime, "vomet", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_LightLime, "unripe lemon", c_VeryRare, Tastiness::Terrible);

	add_flavour(cstr_DarkLime, "pesto", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_DarkLime, "asparagus", c_Rare, Tastiness::Questionable);

	add_flavour(cstr_Chartreuse, "lime", c_Common, Tastiness::Good);
	add_flavour(cstr_Chartreuse, "kiwi", c_Uncommon, Tastiness::Good);
	add_flavour(cstr_Chartreuse, "olive", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_Chartreuse, "green pepper", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_Chartreuse, "ectoplasm", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_Chartreuse, "cactus", c_VeryRare, Tastiness::Questionable);

	add_flavour(cstr_LightChartreuse, "pear", c_Rare, Tastiness::Good);
	add_flavour(cstr_LightChartreuse, "basil", c_Rare, Tastiness::Good);
	add_flavour(cstr_LightChartreuse, "cabbage", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_LightChartreuse, "bok choy", c_VeryRare, Tastiness::Questionable);
	add_flavour(cstr_LightChartreuse, "gooseberry", c_VeryRare, Tastiness::Good);
	add_flavour(cstr_LightChartreuse, "honeydew", c_VeryRare, Tastiness::Good);

	add_flavour(cstr_DarkChartreuse, "avocodo", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_DarkChartreuse, "thyme", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_DarkChartreuse, "dill pickle", c_Rare, Tastiness::Questionable);

	add_flavour(cstr_Green, "parsley", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_Green, "cucumber", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_Green, "grass", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_Green, "green blueberry", c_VeryRare, Tastiness::Questionable);
	add_flavour(cstr_Green, "frog", c_VeryRare, Tastiness::Terrible);

	add_flavour(cstr_LightGreen, "mint", c_Common, Tastiness::Good);
	add_flavour(cstr_LightGreen, "celery", c_Common, Tastiness::Questionable);
	add_flavour(cstr_LightGreen, "cilantro", c_VeryRare, Tastiness::Questionable);

	add_flavour(cstr_DarkerGreen, "broccoli", c_Uncommon, Tastiness::Questionable);
	add_flavour(cstr_DarkerGreen, "spinach", c_Uncommon, Tastiness::Questionable);
	add_flavour(cstr_DarkerGreen, "courgette", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_DarkerGreen, "seaweed", c_Rare, Tastiness::Questionable);
	
	add_flavour(cstr_Sea, "seawater", c_VeryRare, Tastiness::Questionable);

	add_flavour(cstr_LightSea, "spearmint", c_VeryRare, Tastiness::Good);
	add_flavour(cstr_LightSea, "blue cheese", c_VeryRare, Tastiness::Questionable);
	add_flavour(cstr_LightSea, "absinthe", c_VeryRare, Tastiness::Questionable);

	add_flavour(cstr_DarkSea, "lettuce", c_Uncommon, Tastiness::Questionable);
	add_flavour(cstr_DarkSea, "kelp", c_VeryRare, Tastiness::Questionable);

	add_flavour(cstr_Turquoise, "sagebrush", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_Turquoise, "flobberworm", c_VeryRare, Tastiness::Terrible);

	// cyan

	add_flavour(cstr_Sky, "sardine", c_VeryRare, Tastiness::Questionable);
	add_flavour(cstr_Sky, "blue banana", c_VeryRare, Tastiness::Good);

	add_flavour(cstr_LightSky, "ice cream", c_VeryRare, Tastiness::Wonderful);

	add_flavour(cstr_Azure, "blue raspberry", c_Rare, Tastiness::Good);

	add_flavour(cstr_LightAzure, "toothpaste", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_LightAzure, "water", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_LightAzure, "swordfish", c_VeryRare, Tastiness::Questionable);

	add_flavour(cstr_LightBlue, "blueberry", c_Common, Tastiness::Wonderful);
	add_flavour(cstr_LightBlue, "blue strawberry", c_VeryRare, Tastiness::Questionable);
	add_flavour(cstr_LightBlue, "blue tomato", c_VeryRare, Tastiness::Questionable);
	add_flavour(cstr_LightBlue, "spirulina", c_VeryRare, Tastiness::Good);

	add_flavour(cstr_Han, "blackberry", c_Uncommon, Tastiness::Good);
	add_flavour(cstr_Han, "aubergine", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_Han, "blue corn", c_VeryRare, Tastiness::Good);
	add_flavour(cstr_Han, "haskap", c_VeryRare, Tastiness::Questionable);
	add_flavour(cstr_Han, "Saskatoon berry", c_VeryRare, Tastiness::Wonderful);
	add_flavour(cstr_Han, "dirty socks", c_VeryRare, Tastiness::Terrible);
	
	add_flavour(cstr_Violet, "violet", c_Rare, Tastiness::Good);
	add_flavour(cstr_Violet, "plum", c_Uncommon, Tastiness::Good);

	add_flavour(cstr_Purple, "grape", c_Common, Tastiness::Good);
	add_flavour(cstr_Purple, "grape jelly", c_VeryRare, Tastiness::Good);

	add_flavour(cstr_LightPurple, "lavender", c_Rare, Tastiness::Good);
	add_flavour(cstr_LightPurple, "sweet potato", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_LightPurple, "garlic", c_Rare, Tastiness::Questionable);

	//add_flavour(cstr_fuchsia, "fuchsia", c_Common, Tastiness::Questionable);

	add_flavour(cstr_LightFuchsia, "onion", c_Rare, Tastiness::Questionable);

	add_flavour(cstr_DarkFuchsia, "pansy", c_VeryRare, Tastiness::Questionable);

	add_flavour(cstr_Magenta, "dragonfruit", c_VeryRare, Tastiness::Good);

	add_flavour(cstr_LightMagenta, "bubble gum", c_Rare, Tastiness::Good);

	add_flavour(cstr_DarkMagenta, "cranberry", c_Rare, Tastiness::Good);
	add_flavour(cstr_DarkMagenta, "mulberry", c_Rare, Tastiness::Good);
	add_flavour(cstr_DarkMagenta, "elderberry", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_DarkMagenta, "wine", c_Rare, Tastiness::Good);

	// pink

	add_flavour(cstr_LightPink, "candyfloss", c_Uncommon, Tastiness::Good);
	add_flavour(cstr_LightPink, "octopus", c_VeryRare, Tastiness::Terrible);

	add_flavour(cstr_Crimson, "rhubarb", c_Rare, Tastiness::Good);
	add_flavour(cstr_Crimson, "loganberry", c_VeryRare, Tastiness::Good);

	add_flavour(cstr_LightCrimson, "pink grapefruit", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_LightCrimson, "prawn", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_LightCrimson, "salmon", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_LightCrimson, "lipstick", c_VeryRare, Tastiness::Terrible);

	add_flavour(cstr_DarkCrimson, "cherry", c_Common, Tastiness::Wonderful);
	add_flavour(cstr_DarkCrimson, "beet", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_DarkCrimson, "kidney bean", c_Rare, Tastiness::Questionable);

	// If you add flavours, increase the vector reservation.
	assert(Util::Size(s_flavours) <= c_BeanReserveSize);
}

int random_flavour()
{
	return Random::weighted_index(s_weights);
}

char const* get_name(int flavour)
{
	if (Util::IsValidIndex(s_flavours, flavour))
	{
		return s_flavours[flavour].name;
	}
	DebugBreak();
	return "";
}

char const* get_colour(int flavour)
{
	if (Util::IsValidIndex(s_flavours, flavour))
	{
		return s_flavours[flavour].colour;
	}
	DebugBreak();
	return "";
}

std::string get_name_capitalized(int flavour)
{
	std::string s = get_name(flavour);
	s[0] = toupper(s[0]);
	return s;
}

std::string eat_message(int flavour)
{
	BeanFlavour bf = s_flavours[flavour];

	char const* fmt = "";
	bool capitalize = false;

	switch (bf.tastiness)
	{
		case Tastiness::Wonderful:
		{
			int roll = Random::in_range(0,3);
			switch (roll)
			{
				case 0:
					fmt = "Mmm, {}!";
					break;
				case 1:
					fmt = "Ah... {}!";
					break;
				case 2:
					fmt = "{}, delicious!";
					capitalize = true;
					break;
				case 3:
					fmt = "A lovely {} flavour.";
					break;
			}
			break;
		}

		case Tastiness::Good:
		{
			int roll = Random::in_range(0,2);
			switch (roll)
			{
				case 0:
					fmt = "Not bad, it's {} flavour.";
					break;
				case 1:
					fmt = "It tastes like {}.";
					break;
				case 2:
					fmt = "A nice {} flavour.";
					break;
				case 3:
					fmt = "It tastes pretty good.";
					break;
			}
			break;
		}

		case Tastiness::Questionable:
		{
			int roll = Random::in_range(0,2);
			switch (roll)
			{
				case 0:
					fmt = "Was that... {}?";
					break;
				case 1:
					fmt = "Seems to be... {}?";
					break;
				case 2:
					fmt = "{} flavour?";
					capitalize = true;
					break;
				case 3:
					fmt = "It tastes a little strange.";
					break;
			}
			break;
		}

		case Tastiness::Terrible:
		{
			int roll = Random::in_range(0,2);
			switch (roll)
			{
				case 0:
					fmt = "Blech!  It tastes like {}!";
					break;
				case 1:
					fmt = "Alas!  {}!";
					capitalize = true;
					break;
				case 2:
					fmt = "{} flavour!?";
					capitalize = true;
					break;
				case 3:
					fmt = "It tastes horrible!";
					break;
			}
			break;
		}

		default:
			DebugBreak();
	}

	std::string name = (capitalize) ? get_name_capitalized(flavour) : bf.name;
	return std::vformat(fmt, std::make_format_args(name));
}

} // namespace BertieBotts
