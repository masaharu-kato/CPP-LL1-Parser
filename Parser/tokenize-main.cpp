#include "tokenize.h"

//	
//	Rules
//	
//constexpr auto cmap = CharMap('A');
//constexpr auto cmaps = Pattern<1>(cmap);
//constexpr auto CHARS_TEST = NamedPattern<1>(cmaps);
//constexpr auto CHARS_TEST = NamedPattern<1>('A');
constexpr auto CHARS_UPPER_ALPHA = char_range('A', 'Z');
constexpr auto CHARS_LOWER_ALPHA = char_range('a', 'z');
constexpr auto CHARS_DIGIT = char_range('0', '9');
constexpr auto CHARS_ALPHA = CHARS_UPPER_ALPHA | CHARS_LOWER_ALPHA;
constexpr auto CHARS_ALWORD = CHARS_ALPHA | '_';
constexpr auto CHARS_ALNUM = CHARS_DIGIT | CHARS_ALPHA;
constexpr auto CHARS_WORD = CHARS_ALNUM | '_';
constexpr auto CHARS_SYMBOL = Pattern<1>('+', '-', '*', '/', '%', '&', '|', '^', '~', '<', '>', '=', '!', '?', '#', '$', '@', ':', ';');
constexpr auto CHARS_BRACKET = Pattern<1>('(', ')', '[', ']', '{', '}');
constexpr auto CHARS_ESCAPE = Pattern<1>('\\');
constexpr auto CHARS_DQUOTE = Pattern<1>('"');
constexpr auto CHARS_NON_DQUOTE = char_range(1, '"'-1) | char_range('"'+1, '\\'-1) | char_range('\\'+1, 127);
constexpr auto CHARS_SQUOTE = Pattern<1>('\'');
constexpr auto CHARS_NON_SQUOTE = char_range(1, '\"'-1) | char_range('\"'+1, '\\'-1) | char_range('\\'+1, 127);

constexpr auto BRACKET = named("BRACKET", CHARS_BRACKET);
constexpr auto SYMBOL  = named("SYMBOL", CHARS_SYMBOL);
constexpr auto INTEGER = named("INTEGER", +CHARS_DIGIT);
constexpr auto WORD    = named("WORD", (CHARS_ALWORD, +CHARS_WORD));
constexpr auto DQUOTED = named("DQUOTED", (
	CHARS_DQUOTE,
	*(CHARS_NON_DQUOTE | (CHARS_ESCAPE, CHARS_DQUOTE)),
	CHARS_DQUOTE
));
constexpr auto SQUOTED = named("SQUOTED", (
	CHARS_SQUOTE,
	*(CHARS_NON_SQUOTE | (CHARS_ESCAPE, CHARS_SQUOTE)),
	CHARS_SQUOTE
));

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

//int main(void) {
//	return 0;
//}
