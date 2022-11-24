#include "tokenize.h"

//	
//	Rules
//	

constexpr auto CHARS_UPPER_ALPHA = _chars_range<'A', 'Z'>();
constexpr auto CHARS_LOWER_ALPHA = _chars_range<'a', 'z'>();
constexpr auto CHARS_DIGIT = _chars_range<'0', '9'>();
constexpr auto CHARS_ALPHA = _concat_array(CHARS_UPPER_ALPHA, CHARS_LOWER_ALPHA);
constexpr auto CHARS_ALWORD = _concat_array(CHARS_ALPHA, _chars('_'));
constexpr auto CHARS_ALNUM = _concat_array(CHARS_DIGIT, CHARS_ALPHA);
constexpr auto CHARS_WORD = _concat_array(CHARS_ALNUM, _chars('_'));
constexpr auto CHARS_SYMBOL = _chars('+', '-', '*', '/', '%', '&', '|', '^', '~', '<', '>', '=', '!', '?', '#', '$', '@', ':', ';');
constexpr auto CHARS_BRACKET = _chars('(', ')', '[', ']', '{', '}');
constexpr auto CHARS_ESCAPE = _chars('\\');
constexpr auto CHARS_DQUOTE = _chars('"');
constexpr auto CHARS_NON_DQUOTE = _concat_array(_concat_array(_chars_range<1, '"'-1>(), _chars_range<'"'+1, '\\'-1>()), _chars_range<'\\'+1, 127>());
constexpr auto CHARS_SQUOTE = _chars('\'');
constexpr auto CHARS_NON_SQUOTE = _concat_array(_concat_array(_chars_range<1, '\''-1>(), _chars_range<'\''+1, '\\'-1>()), _chars_range<'\\'+1, 127>());

constexpr auto BRACKET = named("BRACKET", one(CHARS_BRACKET));
constexpr auto SYMBOL  = named("SYMBOL", one(CHARS_SYMBOL));
constexpr auto INTEGER = named("INTEGER", repeat(one(CHARS_DIGIT)));
constexpr auto WORD    = named("WORD", concat(one(CHARS_ALWORD), repeat(CHARS_WORD)));
constexpr auto DQUOTED  = named("DQUOTED", concat(
	one(CHARS_DQUOTE),
	opt(repeat(_or(
		one(CHARS_NON_DQUOTE),
		concat(one(CHARS_ESCAPE), one(CHARS_DQUOTE))
	))),
	one(CHARS_DQUOTE))
);
constexpr auto SQUOTED  = named("SQUOTED", concat(
	one(CHARS_SQUOTE),
	opt(repeat(_or(
		one(CHARS_NON_SQUOTE),
		concat(one(CHARS_ESCAPE), one(CHARS_SQUOTE))
	))),
	one(CHARS_SQUOTE))
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
