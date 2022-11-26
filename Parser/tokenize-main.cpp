#include "tokenize.h"

//	
//	Rules
//	

constexpr auto CHARS_UPPER_ALPHA = chars_range<'A', 'Z'>();
constexpr auto CHARS_LOWER_ALPHA = chars_range<'a', 'z'>();
constexpr auto CHARS_DIGIT = chars_range<'0', '9'>();
constexpr auto CHARS_ALPHA = or_(CHARS_UPPER_ALPHA, CHARS_LOWER_ALPHA);
constexpr auto CHARS_ALWORD = or_(CHARS_ALPHA, char_('_'));
constexpr auto CHARS_ALNUM = or_(CHARS_DIGIT, CHARS_ALPHA);
constexpr auto CHARS_WORD = or_(CHARS_ALNUM, char_('_'));
constexpr auto CHARS_SYMBOL = chars('+', '-', '*', '/', '%', '&', '|', '^', '~', '<', '>', '=', '!', '?', '#', '$', '@', ':', ';');
constexpr auto CHARS_BRACKET = chars('(', ')', '[', ']', '{', '}');
constexpr auto CHARS_ESCAPE = char_('\\');
constexpr auto CHARS_DQUOTE = char_('"');
constexpr auto CHARS_NON_DQUOTE = or_(chars_range<1, '"'-1>(), chars_range<'"'+1, '\\'-1>(), chars_range<'\\'+1, 127>());
constexpr auto CHARS_SQUOTE = char_('\'');
constexpr auto CHARS_NON_SQUOTE = or_(chars_range<1, '\''-1>(), chars_range<'\''+1, '\\'-1>(), chars_range<'\\'+1, 127>());

constexpr auto BRACKET = named("BRACKET", CHARS_BRACKET);
constexpr auto SYMBOL  = named("SYMBOL", CHARS_SYMBOL);
constexpr auto INTEGER = named("INTEGER", repeat(CHARS_DIGIT));
constexpr auto WORD    = named("WORD", concat(CHARS_ALWORD, repeat(CHARS_WORD)));
constexpr auto DQUOTED = named("DQUOTED", concat(
	CHARS_DQUOTE,
	opt(repeat(or_(
		CHARS_NON_DQUOTE,
		concat(CHARS_ESCAPE, CHARS_DQUOTE)
	))),
	CHARS_DQUOTE)
);
constexpr auto SQUOTED = named("SQUOTED", concat(
	CHARS_SQUOTE,
	opt(repeat(or_(
		CHARS_NON_SQUOTE,
		concat(CHARS_ESCAPE, CHARS_SQUOTE)
	))),
	CHARS_SQUOTE)
);

constexpr auto pattern_map = make_pattern_map(BRACKET, SYMBOL, INTEGER, WORD, DQUOTED, SQUOTED);



int main(void) {
	const char* text = "hoGe 25*32(25 pw\"\"fi_y)ao**o1_23/h'wye\"f\\'\"ef'rf+25Pi_yo\"this (is) \\\"quo-ted\\\"123\"ends!";
	auto tokens = pattern_map.tokenize(text);
	std::cout << "tokens:" << std::endl;
	for (const auto& token : tokens) {
		std::cout << token << std::endl;
	}
	return 0;
}
