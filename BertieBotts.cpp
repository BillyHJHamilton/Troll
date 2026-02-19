#include "BertieBotts.h"
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
	s_flavours.clear();
	s_weights.clear();

	int constexpr c_Common = 30;
	int constexpr c_Uncommon = 10;
	int constexpr c_Rare = 5;
	int constexpr c_VeryRare = 1;

	char const* cstr_white = "white";
	add_flavour(cstr_white, "coconut", c_Common, Tastiness::Good);
	add_flavour(cstr_white, "milk", c_Uncommon, Tastiness::Questionable);
	add_flavour(cstr_white, "yoghurt", c_Rare, Tastiness::Good);
	add_flavour(cstr_white, "salt", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_white, "sugar", c_Rare, Tastiness::Good);
	add_flavour(cstr_white, "cauliflower", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_white, "lychee", c_VeryRare, Tastiness::Good);
	add_flavour(cstr_white, "white blackberry", c_VeryRare, Tastiness::Questionable);
	add_flavour(cstr_white, "marizpan", c_VeryRare, Tastiness::Wonderful);
	add_flavour(cstr_white, "snow", c_VeryRare, Tastiness::Questionable);

	char const* cstr_light_grey = "lighter grey";
	add_flavour(cstr_light_grey, "vanilla", c_Common, Tastiness::Wonderful);
	add_flavour(cstr_light_grey, "potato", c_Uncommon, Tastiness::Questionable);
	add_flavour(cstr_light_grey, "rice", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_light_grey, "envelope glue", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_light_grey, "dust bunny", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_light_grey, "soap", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_light_grey, "wool", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_light_grey, "sweat", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_light_grey, "slug", c_VeryRare, Tastiness::Terrible);

	char const* cstr_grey = "grey";
	add_flavour(cstr_grey, "mushroom", c_Rare, Tastiness::Good);
	add_flavour(cstr_grey, "sunflower seed", c_Rare, Tastiness::Good);
	add_flavour(cstr_grey, "oyster", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_grey, "cod liver oil", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_grey, "lint", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_grey, "tinfoil", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_grey, "melted plastic", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_grey, "haggis", c_VeryRare, Tastiness::Terrible);

	char const* cstr_dark_grey = "darker grey";
	add_flavour(cstr_dark_grey, "liquorice", c_Common, Tastiness::Good);
	add_flavour(cstr_dark_grey, "sesame", c_Rare, Tastiness::Good);
	add_flavour(cstr_dark_grey, "pepper", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_dark_grey, "black olive", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_dark_grey, "charcoal", c_Rare, Tastiness::Terrible);
	add_flavour(cstr_dark_grey, "black pudding", c_VeryRare, Tastiness::Terrible);

	char const* cstr_red = "red";
	add_flavour(cstr_red, "strawberry", c_Common, Tastiness::Wonderful);
	add_flavour(cstr_red, "raspberry", c_Uncommon, Tastiness::Wonderful);
	add_flavour(cstr_red, "peppermint", c_Uncommon, Tastiness::Good);
	add_flavour(cstr_red, "apple", c_Uncommon, Tastiness::Good);
	add_flavour(cstr_red, "red currant", c_VeryRare, Tastiness::Questionable);

	char const* cstr_light_red = "light red";
	add_flavour(cstr_light_red, "tomato", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_light_red, "fig", c_Rare, Tastiness::Good);
	add_flavour(cstr_light_red, "radish", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_light_red, "watermelon", c_Rare, Tastiness::Wonderful);
	add_flavour(cstr_light_red, "lobster", c_VeryRare, Tastiness::Questionable);
	add_flavour(cstr_light_red, "cinderblock", c_VeryRare, Tastiness::Terrible);

	char const* cstr_dark_red = "dark red";
	add_flavour(cstr_dark_red, "paprika", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_dark_red, "ketchup", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_dark_red, "rose", c_VeryRare, Tastiness::Good);
	add_flavour(cstr_dark_red, "dried blood", c_VeryRare, Tastiness::Terrible);

	char const* cstr_flame = "flame";
	add_flavour(cstr_flame, "pizza", c_Rare, Tastiness::Good);
	add_flavour(cstr_flame, "cream soda", c_Rare, Tastiness::Wonderful);
	add_flavour(cstr_flame, "blood orange", c_VeryRare, Tastiness::Good);
	add_flavour(cstr_flame, "saffron", c_VeryRare,	Tastiness::Good);
	add_flavour(cstr_flame, "rusty metal", c_VeryRare, Tastiness::Terrible);

	char const* cstr_light_flame = "light flame";
	add_flavour(cstr_light_flame, "peach", c_Uncommon, Tastiness::Wonderful);

	char const* cstr_dark_flame = "dark flame";
	add_flavour(cstr_dark_flame, "cinnamon", c_Uncommon, Tastiness::Good);
	add_flavour(cstr_dark_flame, "black tea", c_Rare, Tastiness::Good);
	add_flavour(cstr_dark_flame, "chili powder", c_VeryRare, Tastiness::Questionable);
	add_flavour(cstr_dark_flame, "leather", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_dark_flame, "mud", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_dark_flame, "plum pudding", c_VeryRare, Tastiness::Good);

	char const* cstr_orange = "orange";
	add_flavour(cstr_orange, "orange", c_Common, Tastiness::Good);
	add_flavour(cstr_orange, "carrot", c_Uncommon, Tastiness::Questionable);
	add_flavour(cstr_orange, "marmalade", c_Uncommon, Tastiness::Good);
	add_flavour(cstr_orange, "pumpkin", c_Uncommon, Tastiness::Questionable);
	add_flavour(cstr_orange, "rutabaga", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_orange, "papaya", c_Rare, Tastiness::Good);
	add_flavour(cstr_orange, "apricot", c_Rare, Tastiness::Good);
	add_flavour(cstr_orange, "sawdust", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_orange, "persimmon", c_VeryRare, Tastiness::Good);
	add_flavour(cstr_orange, "kumquat", c_VeryRare, Tastiness::Good);
	add_flavour(cstr_orange, "orange watermelon", c_VeryRare, Tastiness::Good);

	char const* cstr_light_orange = "light orange";
	add_flavour(cstr_light_orange, "toffee", c_Uncommon, Tastiness::Good);
	add_flavour(cstr_light_orange, "spaghetti", c_Rare, Tastiness::Good);
	add_flavour(cstr_light_orange, "oatmeal", c_Rare, Tastiness::Good);
	add_flavour(cstr_light_orange, "caramel", c_Rare, Tastiness::Wonderful);
	add_flavour(cstr_light_orange, "canteloupe", c_Rare, Tastiness::Good);
	add_flavour(cstr_light_orange, "peanut butter", c_Rare, Tastiness::Good);
	add_flavour(cstr_light_orange, "toast", c_Rare, Tastiness::Good);
	add_flavour(cstr_light_orange, "horseradish", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_light_orange, "croissant", c_VeryRare, Tastiness::Good);
	add_flavour(cstr_light_orange, "garbanzo bean", c_VeryRare, Tastiness::Questionable);
	add_flavour(cstr_light_orange, "parmesan", c_VeryRare, Tastiness::Good);
	add_flavour(cstr_light_orange, "earwax", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_light_orange, "tripe", c_VeryRare, Tastiness::Terrible);

	char const* cstr_dark_orange = "darker orange";
	add_flavour(cstr_dark_orange, "chocolate", c_Common, Tastiness::Wonderful);
	add_flavour(cstr_dark_orange, "coffee", c_Uncommon, Tastiness::Good);
	add_flavour(cstr_dark_orange, "lentil", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_dark_orange, "baked bean", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_dark_orange, "dirt", c_Rare, Tastiness::Terrible);
	add_flavour(cstr_dark_orange, "dog food", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_dark_orange, "haggis", c_VeryRare, Tastiness::Terrible);

	char const* cstr_amber = "amber";
	add_flavour(cstr_amber, "honey", c_Rare, Tastiness::Wonderful);
	add_flavour(cstr_amber, "mango", c_Rare, Tastiness::Wonderful);

	char const* cstr_light_amber = "light amber";
	add_flavour(cstr_light_amber, "almond", c_Rare, Tastiness::Good);
	add_flavour(cstr_light_amber, "fried egg", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_light_amber, "champagne", c_VeryRare, Tastiness::Good);

	char const* cstr_yellow = "yellow";
	add_flavour(cstr_yellow, "lemon", c_Common, Tastiness::Good);
	add_flavour(cstr_yellow, "pineapple", c_Uncommon, Tastiness::Good);
	add_flavour(cstr_yellow, "sour lemon", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_yellow, "lemonade", c_VeryRare, Tastiness::Wonderful);
	add_flavour(cstr_yellow, "highlighter ink", c_VeryRare, Tastiness::Terrible);

	char const* cstr_light_yellow = "light yellow";
	add_flavour(cstr_light_yellow, "banana", c_Common, Tastiness::Good);
	add_flavour(cstr_light_yellow, "popcorn", c_Uncommon, Tastiness::Good);
	add_flavour(cstr_light_yellow, "corn", c_Uncommon, Tastiness::Good);
	add_flavour(cstr_light_yellow, "butter", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_light_yellow, "cheese", c_Rare, Tastiness::Good);
	add_flavour(cstr_light_yellow, "custard", c_VeryRare, Tastiness::Wonderful);

	char const* cstr_dark_yellow = "dark yellow";
	add_flavour(cstr_dark_yellow, "mustard", c_Uncommon, Tastiness::Questionable);
	add_flavour(cstr_dark_yellow, "passionfruit", c_Rare, Tastiness::Good);
	add_flavour(cstr_dark_yellow, "curry", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_dark_yellow, "old banana", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_dark_yellow, "sulphur", c_VeryRare, Tastiness::Terrible);

	char const* cstr_lime = "lime";
	add_flavour(cstr_lime, "bogey", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_lime, "chlorine", c_VeryRare, Tastiness::Terrible);

	char const* cstr_light_lime = "light lime";
	add_flavour(cstr_light_lime, "green apple", c_Uncommon, Tastiness::Good);
	add_flavour(cstr_light_lime, "pistachio", c_VeryRare, Tastiness::Good);
	add_flavour(cstr_light_lime, "sprouts", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_light_lime, "green tea", c_Rare, Tastiness::Good);
	add_flavour(cstr_light_lime, "vomet", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_light_lime, "unripe lemon", c_VeryRare, Tastiness::Terrible);

	char const* cstr_dark_lime = "dark lime";
	add_flavour(cstr_dark_lime, "pesto", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_dark_lime, "asparagus", c_Rare, Tastiness::Questionable);

	char const* cstr_chartreuse = "chartreuse";
	add_flavour(cstr_chartreuse, "lime", c_Common, Tastiness::Good);
	add_flavour(cstr_chartreuse, "kiwi", c_Uncommon, Tastiness::Good);
	add_flavour(cstr_chartreuse, "olive", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_chartreuse, "green pepper", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_chartreuse, "ectoplasm", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_chartreuse, "cactus", c_VeryRare, Tastiness::Questionable);

	char const* cstr_light_chartreuse = "light chartreuse";
	add_flavour(cstr_light_chartreuse, "pear", c_Rare, Tastiness::Good);
	add_flavour(cstr_light_chartreuse, "basil", c_Rare, Tastiness::Good);
	add_flavour(cstr_light_chartreuse, "cabbage", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_light_chartreuse, "bok choy", c_VeryRare, Tastiness::Questionable);
	add_flavour(cstr_light_chartreuse, "gooseberry", c_VeryRare, Tastiness::Good);
	add_flavour(cstr_light_chartreuse, "honeydew", c_VeryRare, Tastiness::Good);

	char const* cstr_dark_chartreuse = "dark chartreuse";
	add_flavour(cstr_dark_chartreuse, "avocodo", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_dark_chartreuse, "thyme", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_dark_chartreuse, "dill pickle", c_Rare, Tastiness::Questionable);

	char const* cstr_green = "green";
	add_flavour(cstr_green, "parsley", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_green, "cucumber", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_green, "grass", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_green, "green blueberry", c_VeryRare, Tastiness::Questionable);
	add_flavour(cstr_green, "frog", c_VeryRare, Tastiness::Terrible);

	char const* cstr_light_green = "light green";
	add_flavour(cstr_light_green, "mint", c_Common, Tastiness::Good);
	add_flavour(cstr_light_green, "celery", c_Common, Tastiness::Questionable);
	add_flavour(cstr_light_green, "cilantro", c_VeryRare, Tastiness::Questionable);

	char const* cstr_dark_green = "darker green";
	add_flavour(cstr_dark_green, "broccoli", c_Uncommon, Tastiness::Questionable);
	add_flavour(cstr_dark_green, "spinach", c_Uncommon, Tastiness::Questionable);
	add_flavour(cstr_dark_green, "courgette", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_dark_green, "seaweed", c_Rare, Tastiness::Questionable);
	
	char const* cstr_sea = "sea";
	add_flavour(cstr_sea, "seawater", c_VeryRare, Tastiness::Questionable);

	char const* cstr_light_sea = "light sea";
	add_flavour(cstr_light_sea, "spearmint", c_VeryRare, Tastiness::Good);
	add_flavour(cstr_light_sea, "blue cheese", c_VeryRare, Tastiness::Questionable);
	add_flavour(cstr_light_sea, "absinthe", c_VeryRare, Tastiness::Questionable);

	char const* cstr_dark_sea = "dark sea";
	add_flavour(cstr_dark_sea, "lettuce", c_Uncommon, Tastiness::Questionable);
	add_flavour(cstr_dark_sea, "kelp", c_VeryRare, Tastiness::Questionable);

	char const* cstr_turquoise = "turquoise";
	add_flavour(cstr_turquoise, "sagebrush", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_turquoise, "flobberworm", c_VeryRare, Tastiness::Terrible);

	//char const* cstr_cyan = "cyan";

	char const* cstr_sky = "sky";
	add_flavour(cstr_sky, "sardine", c_VeryRare, Tastiness::Questionable);
	add_flavour(cstr_sky, "blue banana", c_VeryRare, Tastiness::Good);

	char const* cstr_light_sky = "light sky";
	add_flavour(cstr_light_sky, "ice cream", c_VeryRare, Tastiness::Wonderful);

	char const* cstr_azure = "azure";
	add_flavour(cstr_azure, "blue raspberry", c_Rare, Tastiness::Good);

	char const* cstr_light_azure = "light azure";
	add_flavour(cstr_light_azure, "toothpaste", c_VeryRare, Tastiness::Terrible);
	add_flavour(cstr_light_azure, "water", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_light_azure, "swordfish", c_VeryRare, Tastiness::Questionable);

	char const* cstr_light_blue = "light blue";
	add_flavour(cstr_light_blue, "blueberry", c_Common, Tastiness::Wonderful);
	add_flavour(cstr_light_blue, "blue strawberry", c_VeryRare, Tastiness::Questionable);
	add_flavour(cstr_light_blue, "blue tomato", c_VeryRare, Tastiness::Questionable);
	add_flavour(cstr_light_blue, "spirulina", c_VeryRare, Tastiness::Good);

	char const* cstr_han = "han";
	add_flavour(cstr_han, "blackberry", c_Uncommon, Tastiness::Good);
	add_flavour(cstr_han, "aubergine", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_han, "blue corn", c_VeryRare, Tastiness::Good);
	add_flavour(cstr_han, "haskap", c_VeryRare, Tastiness::Questionable);
	add_flavour(cstr_han, "Saskatoon berry", c_VeryRare, Tastiness::Wonderful);
	add_flavour(cstr_han, "dirty socks", c_VeryRare, Tastiness::Terrible);
	
	char const* cstr_violet = "violet";
	add_flavour(cstr_violet, "violet", c_Rare, Tastiness::Good);
	add_flavour(cstr_violet, "plum", c_Uncommon, Tastiness::Good);

	char const* cstr_purple = "purple";
	add_flavour(cstr_purple, "grape", c_Common, Tastiness::Good);
	add_flavour(cstr_purple, "grape jelly", c_VeryRare, Tastiness::Good);

	char const* cstr_light_purple = "light purple";
	add_flavour(cstr_light_purple, "lavender", c_Rare, Tastiness::Good);
	add_flavour(cstr_light_purple, "sweet potato", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_light_purple, "garlic", c_Rare, Tastiness::Questionable);

	//char const* cstr_fuchsia = "fuchsia";
	//add_flavour(cstr_fuchsia, "fuchsia", c_Common, Tastiness::Questionable);

	char const* cstr_light_fuchsia = "light fuchsia";
	add_flavour(cstr_light_fuchsia, "onion", c_Rare, Tastiness::Questionable);

	char const* cstr_dark_fuchsia = "dark fuchsia";
	add_flavour(cstr_dark_fuchsia, "pansy", c_VeryRare, Tastiness::Questionable);

	char const* cstr_magenta = "magenta";
	add_flavour(cstr_magenta, "dragonfruit", c_VeryRare, Tastiness::Good);

	char const* cstr_light_magenta = "light magenta";
	add_flavour(cstr_light_magenta, "bubble gum", c_Rare, Tastiness::Good);

	char const* cstr_dark_magenta = "dark magenta";
	add_flavour(cstr_dark_magenta, "cranberry", c_Rare, Tastiness::Good);
	add_flavour(cstr_dark_magenta, "mulberry", c_Rare, Tastiness::Good);
	add_flavour(cstr_dark_magenta, "elderberry", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_dark_magenta, "wine", c_Rare, Tastiness::Good);

	//char const* cstr_pink = "pink";

	char const* cstr_light_pink = "light pink";
	add_flavour(cstr_light_pink, "candyfloss", c_Uncommon, Tastiness::Good);
	add_flavour(cstr_light_pink, "octopus", c_VeryRare, Tastiness::Terrible);

	char const* cstr_crimson = "crimson";
	add_flavour(cstr_crimson, "rhubarb", c_Rare, Tastiness::Good);
	add_flavour(cstr_crimson, "loganberry", c_VeryRare, Tastiness::Good);

	char const* cstr_light_crimson = "light crimson";
	add_flavour(cstr_light_crimson, "pink grapefruit", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_light_crimson, "prawn", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_light_crimson, "salmon", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_light_crimson, "lipstick", c_VeryRare, Tastiness::Terrible);

	char const* cstr_dark_crimson = "dark crimson";
	add_flavour(cstr_dark_crimson, "cherry", c_Common, Tastiness::Wonderful);
	add_flavour(cstr_dark_crimson, "beet", c_Rare, Tastiness::Questionable);
	add_flavour(cstr_dark_crimson, "kidney bean", c_Rare, Tastiness::Questionable);
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
					fmt = " Mmm, {}!";
					break;
				case 1:
					fmt = " Ah... {}!";
					break;
				case 2:
					fmt = " {}, delicious!";
					capitalize = true;
					break;
				case 3:
					fmt = " A lovely {} flavour.";
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
					fmt = " Not bad, it's {} flavour.";
					break;
				case 1:
					fmt = " It tastes like {}.";
					break;
				case 2:
					fmt = " A nice {} flavour.";
					break;
				case 3:
					fmt = " It tastes pretty good.";
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
					fmt = " Was that... {}?";
					break;
				case 1:
					fmt = " Seems to be... {}?";
					break;
				case 2:
					fmt = " {} flavour?";
					capitalize = true;
					break;
				case 3:
					fmt = " It tastes a little strange.";
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
					fmt = " Blech!  It tastes like {}!";
					break;
				case 1:
					fmt = " Alas!  {}!";
					capitalize = true;
					break;
				case 2:
					fmt = " {} flavour!?";
					capitalize = true;
					break;
				case 3:
					fmt = " It tastes horrible!";
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
