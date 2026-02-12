#include "BertieBotts.h"
#include "Debug.h"
#include "Random.h"
#include "VectorUtil.h"

namespace BertieBotts
{

enum class Tastiness
{
	Wonderful,
	Good,
	Questionable,
	Terrible,
	Tasteless
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

	int constexpr c_common = 30;
	int constexpr c_uncommon = 10;
	int constexpr c_rare = 5;
	int constexpr c_very_rare = 1;

	char const* cstr_white = "white";
	add_flavour(cstr_white, "coconut", c_common, Tastiness::Good);
	add_flavour(cstr_white, "milk", c_uncommon, Tastiness::Questionable);
	add_flavour(cstr_white, "yoghurt", c_rare, Tastiness::Good);
	add_flavour(cstr_white, "salt", c_rare, Tastiness::Questionable);
	add_flavour(cstr_white, "sugar", c_rare, Tastiness::Good);
	add_flavour(cstr_white, "cauliflower", c_rare, Tastiness::Questionable);
	add_flavour(cstr_white, "lychee", c_very_rare, Tastiness::Good);
	add_flavour(cstr_white, "white blackberry", c_very_rare, Tastiness::Questionable);
	add_flavour(cstr_white, "marizpan", c_very_rare, Tastiness::Wonderful);
	add_flavour(cstr_white, "snow", c_very_rare, Tastiness::Tasteless);

	char const* cstr_light_grey = "lighter grey";
	add_flavour(cstr_light_grey, "vanilla", c_common, Tastiness::Wonderful);
	add_flavour(cstr_light_grey, "potato", c_uncommon, Tastiness::Questionable);
	add_flavour(cstr_light_grey, "rice", c_rare, Tastiness::Questionable);
	add_flavour(cstr_light_grey, "envelope glue", c_very_rare, Tastiness::Terrible);
	add_flavour(cstr_light_grey, "dust bunny", c_very_rare, Tastiness::Terrible);
	add_flavour(cstr_light_grey, "soap", c_very_rare, Tastiness::Terrible);
	add_flavour(cstr_light_grey, "wool", c_very_rare, Tastiness::Terrible);
	add_flavour(cstr_light_grey, "sweat", c_very_rare, Tastiness::Terrible);
	add_flavour(cstr_light_grey, "slug", c_very_rare, Tastiness::Terrible);

	char const* cstr_grey = "grey";
	add_flavour(cstr_grey, "mushroom", c_rare, Tastiness::Good);
	add_flavour(cstr_grey, "sunflower seed", c_rare, Tastiness::Good);
	add_flavour(cstr_grey, "oyster", c_rare, Tastiness::Questionable);
	add_flavour(cstr_grey, "cod liver oil", c_very_rare, Tastiness::Terrible);
	add_flavour(cstr_grey, "lint", c_very_rare, Tastiness::Terrible);
	add_flavour(cstr_grey, "tinfoil", c_very_rare, Tastiness::Terrible);
	add_flavour(cstr_grey, "melted plastic", c_very_rare, Tastiness::Terrible);
	add_flavour(cstr_grey, "haggis", c_very_rare, Tastiness::Terrible);

	char const* cstr_dark_grey = "darker grey";
	add_flavour(cstr_dark_grey, "liquorice", c_common, Tastiness::Good);
	add_flavour(cstr_dark_grey, "sesame", c_rare, Tastiness::Good);
	add_flavour(cstr_dark_grey, "pepper", c_rare, Tastiness::Questionable);
	add_flavour(cstr_dark_grey, "black olive", c_rare, Tastiness::Questionable);
	add_flavour(cstr_dark_grey, "charcoal", c_rare, Tastiness::Terrible);
	add_flavour(cstr_dark_grey, "black pudding", c_very_rare, Tastiness::Terrible);

	char const* cstr_red = "red";
	add_flavour(cstr_red, "strawberry", c_common, Tastiness::Wonderful);
	add_flavour(cstr_red, "raspberry", c_uncommon, Tastiness::Wonderful);
	add_flavour(cstr_red, "peppermint", c_uncommon, Tastiness::Good);
	add_flavour(cstr_red, "apple", c_uncommon, Tastiness::Good);
	add_flavour(cstr_red, "red currant", c_very_rare, Tastiness::Questionable);

	char const* cstr_light_red = "light red";
	add_flavour(cstr_light_red, "tomato", c_rare, Tastiness::Questionable);
	add_flavour(cstr_light_red, "fig", c_rare, Tastiness::Good);
	add_flavour(cstr_light_red, "radish", c_rare, Tastiness::Questionable);
	add_flavour(cstr_light_red, "watermelon", c_rare, Tastiness::Wonderful);
	add_flavour(cstr_light_red, "lobster", c_very_rare, Tastiness::Questionable);
	add_flavour(cstr_light_red, "cinderblock", c_very_rare, Tastiness::Terrible);

	char const* cstr_dark_red = "dark red";
	add_flavour(cstr_dark_red, "paprika", c_rare, Tastiness::Questionable);
	add_flavour(cstr_dark_red, "ketchup", c_rare, Tastiness::Questionable);
	add_flavour(cstr_dark_red, "rose", c_very_rare, Tastiness::Good);
	add_flavour(cstr_dark_red, "dried blood", c_very_rare, Tastiness::Terrible);

	char const* cstr_flame = "flame";
	add_flavour(cstr_flame, "pizza", c_rare, Tastiness::Good);
	add_flavour(cstr_flame, "cream soda", c_rare, Tastiness::Wonderful);
	add_flavour(cstr_flame, "blood orange", c_very_rare, Tastiness::Good);
	add_flavour(cstr_flame, "saffron", c_very_rare,	Tastiness::Good);
	add_flavour(cstr_flame, "rusty metal", c_very_rare, Tastiness::Terrible);

	char const* cstr_light_flame = "light flame";
	add_flavour(cstr_light_flame, "peach", c_uncommon, Tastiness::Wonderful);

	char const* cstr_dark_flame = "dark flame";
	add_flavour(cstr_dark_flame, "cinnamon", c_uncommon, Tastiness::Good);
	add_flavour(cstr_dark_flame, "black tea", c_rare, Tastiness::Good);
	add_flavour(cstr_dark_flame, "chili powder", c_very_rare, Tastiness::Questionable);
	add_flavour(cstr_dark_flame, "leather", c_very_rare, Tastiness::Terrible);
	add_flavour(cstr_dark_flame, "mud", c_very_rare, Tastiness::Terrible);
	add_flavour(cstr_dark_flame, "plum pudding", c_very_rare, Tastiness::Good);

	char const* cstr_orange = "orange";
	add_flavour(cstr_orange, "orange", c_common, Tastiness::Good);
	add_flavour(cstr_orange, "carrot", c_uncommon, Tastiness::Questionable);
	add_flavour(cstr_orange, "marmalade", c_uncommon, Tastiness::Good);
	add_flavour(cstr_orange, "pumpkin", c_uncommon, Tastiness::Questionable);
	add_flavour(cstr_orange, "rutabaga", c_rare, Tastiness::Questionable);
	add_flavour(cstr_orange, "papaya", c_rare, Tastiness::Good);
	add_flavour(cstr_orange, "apricot", c_rare, Tastiness::Good);
	add_flavour(cstr_orange, "sawdust", c_very_rare, Tastiness::Terrible);
	add_flavour(cstr_orange, "persimmon", c_very_rare, Tastiness::Good);
	add_flavour(cstr_orange, "kumquat", c_very_rare, Tastiness::Good);
	add_flavour(cstr_orange, "orange watermelon", c_very_rare, Tastiness::Good);

	char const* cstr_light_orange = "light orange";
	add_flavour(cstr_light_orange, "toffee", c_uncommon, Tastiness::Good);
	add_flavour(cstr_light_orange, "spaghetti", c_rare, Tastiness::Good);
	add_flavour(cstr_light_orange, "oatmeal", c_rare, Tastiness::Good);
	add_flavour(cstr_light_orange, "caramel", c_rare, Tastiness::Wonderful);
	add_flavour(cstr_light_orange, "canteloupe", c_rare, Tastiness::Good);
	add_flavour(cstr_light_orange, "peanut butter", c_rare, Tastiness::Good);
	add_flavour(cstr_light_orange, "croissant", c_very_rare, Tastiness::Good);
	add_flavour(cstr_light_orange, "garbanzo bean", c_very_rare, Tastiness::Questionable);
	add_flavour(cstr_light_orange, "parmesan", c_very_rare, Tastiness::Good);
	add_flavour(cstr_light_orange, "earwax", c_very_rare, Tastiness::Terrible);
	add_flavour(cstr_light_orange, "tripe", c_very_rare, Tastiness::Terrible);

	char const* cstr_dark_orange = "darker orange";
	add_flavour(cstr_dark_orange, "chocolate", c_common, Tastiness::Wonderful);
	add_flavour(cstr_dark_orange, "coffee", c_uncommon, Tastiness::Good);
	add_flavour(cstr_dark_orange, "lentil", c_rare, Tastiness::Questionable);
	add_flavour(cstr_dark_orange, "dirt", c_rare, Tastiness::Terrible);
	add_flavour(cstr_dark_orange, "dog food", c_very_rare, Tastiness::Terrible);
	add_flavour(cstr_dark_orange, "haggis", c_very_rare, Tastiness::Terrible);

	char const* cstr_amber = "amber";
	add_flavour(cstr_amber, "honey", c_rare, Tastiness::Wonderful);
	add_flavour(cstr_amber, "mango", c_rare, Tastiness::Wonderful);

	char const* cstr_light_amber = "light amber";
	add_flavour(cstr_light_amber, "almond", c_rare, Tastiness::Good);
	add_flavour(cstr_light_amber, "fried egg", c_rare, Tastiness::Questionable);
	add_flavour(cstr_light_amber, "champagne", c_very_rare, Tastiness::Good);

	char const* cstr_yellow = "yellow";
	add_flavour(cstr_yellow, "lemon", c_common, Tastiness::Good);
	add_flavour(cstr_yellow, "pineapple", c_uncommon, Tastiness::Good);
	add_flavour(cstr_yellow, "sour lemon", c_rare, Tastiness::Questionable);
	add_flavour(cstr_yellow, "lemonade", c_very_rare, Tastiness::Wonderful);
	add_flavour(cstr_yellow, "highlighter ink", c_very_rare, Tastiness::Terrible);

	char const* cstr_light_yellow = "light yellow";
	add_flavour(cstr_light_yellow, "banana", c_common, Tastiness::Good);
	add_flavour(cstr_light_yellow, "popcorn", c_uncommon, Tastiness::Good);
	add_flavour(cstr_light_yellow, "corn", c_uncommon, Tastiness::Good);
	add_flavour(cstr_light_yellow, "butter", c_rare, Tastiness::Questionable);
	add_flavour(cstr_light_yellow, "cheese", c_rare, Tastiness::Good);
	add_flavour(cstr_light_yellow, "custard", c_very_rare, Tastiness::Wonderful);

	char const* cstr_dark_yellow = "dark yellow";
	add_flavour(cstr_dark_yellow, "mustard", c_uncommon, Tastiness::Questionable);
	add_flavour(cstr_dark_yellow, "passionfruit", c_rare, Tastiness::Good);
	add_flavour(cstr_dark_yellow, "curry", c_rare, Tastiness::Questionable);
	add_flavour(cstr_dark_yellow, "old banana", c_rare, Tastiness::Questionable);
	add_flavour(cstr_dark_yellow, "sulphur", c_very_rare, Tastiness::Terrible);

	char const* cstr_lime = "lime";
	add_flavour(cstr_lime, "bogey", c_very_rare, Tastiness::Terrible);
	add_flavour(cstr_lime, "chlorine", c_very_rare, Tastiness::Terrible);

	char const* cstr_light_lime = "light lime";
	add_flavour(cstr_light_lime, "green apple", c_uncommon, Tastiness::Good);
	add_flavour(cstr_light_lime, "pistachio", c_very_rare, Tastiness::Good);
	add_flavour(cstr_light_lime, "green tea", c_rare, Tastiness::Good);
	add_flavour(cstr_light_lime, "vomet", c_very_rare, Tastiness::Terrible);
	add_flavour(cstr_light_lime, "unripe lemon", c_very_rare, Tastiness::Terrible);

	char const* cstr_dark_lime = "dark lime";
	add_flavour(cstr_dark_lime, "pesto", c_rare, Tastiness::Questionable);
	add_flavour(cstr_dark_lime, "asparagus", c_rare, Tastiness::Questionable);

	char const* cstr_chartreuse = "chartreuse";
	add_flavour(cstr_chartreuse, "lime", c_common, Tastiness::Good);
	add_flavour(cstr_chartreuse, "kiwi", c_uncommon, Tastiness::Good);
	add_flavour(cstr_chartreuse, "olive", c_rare, Tastiness::Questionable);
	add_flavour(cstr_chartreuse, "green pepper", c_rare, Tastiness::Questionable);
	add_flavour(cstr_chartreuse, "ectoplasm", c_very_rare, Tastiness::Terrible);
	add_flavour(cstr_chartreuse, "cactus", c_very_rare, Tastiness::Questionable);

	char const* cstr_light_chartreuse = "light chartreuse";
	add_flavour(cstr_light_chartreuse, "pear", c_rare, Tastiness::Good);
	add_flavour(cstr_light_chartreuse, "basil", c_rare, Tastiness::Good);
	add_flavour(cstr_light_chartreuse, "cabbage", c_rare, Tastiness::Questionable);
	add_flavour(cstr_light_chartreuse, "bok choy", c_very_rare, Tastiness::Questionable);
	add_flavour(cstr_light_chartreuse, "gooseberry", c_very_rare, Tastiness::Good);
	add_flavour(cstr_light_chartreuse, "honeydew", c_very_rare, Tastiness::Good);

	char const* cstr_dark_chartreuse = "dark chartreuse";
	add_flavour(cstr_dark_chartreuse, "avocodo", c_rare, Tastiness::Questionable);
	add_flavour(cstr_dark_chartreuse, "thyme", c_rare, Tastiness::Questionable);

	char const* cstr_green = "green";
	add_flavour(cstr_green, "parsley", c_rare, Tastiness::Questionable);
	add_flavour(cstr_green, "cucumber", c_rare, Tastiness::Questionable);
	add_flavour(cstr_green, "grass", c_very_rare, Tastiness::Terrible);
	add_flavour(cstr_green, "green blueberry", c_very_rare, Tastiness::Questionable);
	add_flavour(cstr_green, "frog", c_very_rare, Tastiness::Terrible);

	char const* cstr_light_green = "light green";
	add_flavour(cstr_light_green, "mint", c_common, Tastiness::Good);
	add_flavour(cstr_light_green, "celery", c_common, Tastiness::Questionable);
	add_flavour(cstr_light_green, "cilantro", c_very_rare, Tastiness::Questionable);

	char const* cstr_dark_green = "darker green";
	add_flavour(cstr_dark_green, "broccoli", c_uncommon, Tastiness::Questionable);
	add_flavour(cstr_dark_green, "spinach", c_uncommon, Tastiness::Questionable);
	add_flavour(cstr_dark_green, "courgette", c_rare, Tastiness::Questionable);
	add_flavour(cstr_dark_green, "seaweed", c_rare, Tastiness::Questionable);
	
	char const* cstr_sea = "sea";
	add_flavour(cstr_sea, "seawater", c_very_rare, Tastiness::Questionable);

	char const* cstr_light_sea = "light sea";
	add_flavour(cstr_light_sea, "spearmint", c_very_rare, Tastiness::Good);
	add_flavour(cstr_light_sea, "blue cheese", c_very_rare, Tastiness::Questionable);
	add_flavour(cstr_light_sea, "absinthe", c_very_rare, Tastiness::Questionable);

	char const* cstr_dark_sea = "dark sea";
	add_flavour(cstr_dark_sea, "lettuce", c_uncommon, Tastiness::Questionable);
	add_flavour(cstr_dark_sea, "kelp", c_very_rare, Tastiness::Questionable);

	char const* cstr_turquoise = "turquoise";
	add_flavour(cstr_turquoise, "sagebrush", c_rare, Tastiness::Questionable);
	add_flavour(cstr_turquoise, "flobberworm", c_very_rare, Tastiness::Terrible);

	//char const* cstr_cyan = "cyan";

	char const* cstr_sky = "sky";
	add_flavour(cstr_sky, "sardine", c_very_rare, Tastiness::Questionable);
	add_flavour(cstr_sky, "blue banana", c_very_rare, Tastiness::Good);

	char const* cstr_light_sky = "light sky";
	add_flavour(cstr_light_sky, "ice cream", c_very_rare, Tastiness::Wonderful);

	char const* cstr_azure = "azure";
	add_flavour(cstr_azure, "blue raspberry", c_rare, Tastiness::Good);

	char const* cstr_light_azure = "light azure";
	add_flavour(cstr_light_azure, "toothpaste", c_very_rare, Tastiness::Terrible);
	add_flavour(cstr_light_azure, "water", c_rare, Tastiness::Tasteless);
	add_flavour(cstr_light_azure, "swordfish", c_very_rare, Tastiness::Questionable);

	char const* cstr_light_blue = "light blue";
	add_flavour(cstr_light_blue, "blueberry", c_common, Tastiness::Wonderful);
	add_flavour(cstr_light_blue, "blue strawberry", c_very_rare, Tastiness::Questionable);
	add_flavour(cstr_light_blue, "blue tomato", c_very_rare, Tastiness::Questionable);
	add_flavour(cstr_light_blue, "spirulina", c_very_rare, Tastiness::Good);

	char const* cstr_han = "han";
	add_flavour(cstr_han, "blackberry", c_uncommon, Tastiness::Good);
	add_flavour(cstr_han, "aubergine", c_rare, Tastiness::Questionable);
	add_flavour(cstr_han, "blue corn", c_very_rare, Tastiness::Good);
	add_flavour(cstr_han, "haskap", c_very_rare, Tastiness::Questionable);
	add_flavour(cstr_han, "Saskatoon berry", c_very_rare, Tastiness::Wonderful);
	add_flavour(cstr_han, "dirty sock", c_very_rare, Tastiness::Terrible);
	
	char const* cstr_violet = "violet";
	add_flavour(cstr_violet, "violet", c_rare, Tastiness::Good);
	add_flavour(cstr_violet, "plum", c_uncommon, Tastiness::Good);

	char const* cstr_purple = "purple";
	add_flavour(cstr_purple, "grape", c_common, Tastiness::Good);
	add_flavour(cstr_purple, "grape jelly", c_very_rare, Tastiness::Good);

	char const* cstr_light_purple = "light purple";
	add_flavour(cstr_light_purple, "lavender", c_rare, Tastiness::Good);
	add_flavour(cstr_light_purple, "sweet potato", c_rare, Tastiness::Questionable);
	add_flavour(cstr_light_purple, "garlic", c_rare, Tastiness::Questionable);

	//char const* cstr_fuchsia = "fuchsia";
	//add_flavour(cstr_fuchsia, "fuchsia", c_common, Tastiness::Questionable);

	char const* cstr_light_fuchsia = "light fuchsia";
	add_flavour(cstr_light_fuchsia, "onion", c_rare, Tastiness::Questionable);

	char const* cstr_dark_fuchsia = "dark fuchsia";
	add_flavour(cstr_dark_fuchsia, "pansy", c_very_rare, Tastiness::Questionable);

	char const* cstr_magenta = "magenta";
	add_flavour(cstr_magenta, "dragonfruit", c_very_rare, Tastiness::Good);

	char const* cstr_light_magenta = "light magenta";
	add_flavour(cstr_light_magenta, "bubble gum", c_rare, Tastiness::Good);

	char const* cstr_dark_magenta = "dark magenta";
	add_flavour(cstr_dark_magenta, "cranberry", c_rare, Tastiness::Good);
	add_flavour(cstr_dark_magenta, "mulberry", c_rare, Tastiness::Good);
	add_flavour(cstr_dark_magenta, "elderberry", c_rare, Tastiness::Questionable);
	add_flavour(cstr_dark_magenta, "wine", c_rare, Tastiness::Good);

	//char const* cstr_pink = "pink";

	char const* cstr_light_pink = "light pink";
	add_flavour(cstr_light_pink, "candyfloss", c_uncommon, Tastiness::Good);
	add_flavour(cstr_light_pink, "octopus", c_very_rare, Tastiness::Terrible);

	char const* cstr_crimson = "crimson";
	add_flavour(cstr_crimson, "rhubarb", c_rare, Tastiness::Good);
	add_flavour(cstr_crimson, "loganberry", c_very_rare, Tastiness::Good);

	char const* cstr_light_crimson = "light crimson";
	add_flavour(cstr_light_crimson, "pink grapefruit", c_rare, Tastiness::Questionable);
	add_flavour(cstr_light_crimson, "prawn", c_rare, Tastiness::Questionable);
	add_flavour(cstr_light_crimson, "salmon", c_rare, Tastiness::Questionable);
	add_flavour(cstr_light_crimson, "lipstick", c_very_rare, Tastiness::Terrible);

	char const* cstr_dark_crimson = "dark crimson";
	add_flavour(cstr_dark_crimson, "cherry", c_common, Tastiness::Wonderful);
	add_flavour(cstr_dark_crimson, "beet", c_rare, Tastiness::Questionable);
	add_flavour(cstr_dark_crimson, "kidney bean", c_rare, Tastiness::Questionable);
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

} // namespace BertieBotts
