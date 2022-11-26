#pragma once
#include <array>
#include <vector>
#include <string>
#include <iostream>

constexpr size_t NCHAR = 256;
using uchar = unsigned char;

template<typename T, typename... Ts>
constexpr auto _sum(T&& t, Ts&&... ts) { return (std::forward<T>(t) + ... + std::forward<Ts>(ts)); }

template <typename T, typename ...Args>
constexpr auto _make_array(Args&&... args)
{
	return std::array<T, sizeof...(Args)>{args...};
}

template <typename T, size_t L1, size_t L2>
constexpr std::array<T, L1 + L2> _concat_array(std::array<T, L1> a1, std::array<T, L2> a2) {
	std::array<T, L1 + L2> a12{};
	size_t i = 0;
	for (auto v : a1) a12[i++] = v;
	for (auto v : a2) a12[i++] = v;
	return a12;
}

template <typename T, size_t L>
constexpr std::array<T, L> _concat_array(std::array<T, L> a) {
	return a;
}

template <typename T, size_t L1, size_t L2, size_t... Ls>
constexpr std::array<T, _sum(L1, L2, Ls...)> _concat_array(std::array<T, L1> a1, std::array<T, L2> a2, std::array<T, Ls>... arrs) {
	return _concat_array(_concat_array(a1, a2), arrs...);
}


template <typename ...Args>
constexpr std::array<uchar, sizeof...(Args)> _chars(Args... args) {
	return { (uchar)args... };
}

constexpr std::array<uchar, 1> _char(uchar ch) {
	return { ch };
}

template <uchar CB, uchar CE>
constexpr auto _chars_range() {
	std::array<uchar, CE - CB + 1> chars{};
	for (uchar ch = CB; ch <= CE; ++ch) chars[ch - CB] = ch;
	return chars;
}

using CharMap = std::array<int, NCHAR>;
constexpr int CMAP_LAST = -1;	//	The token will ends on the current positon
constexpr int CMAP_END = -2;	//	The token ended on the previous position
constexpr int CMAP_NOT = -3;	//	Invalid character for this token

constexpr CharMap _charmap(int v) {
	CharMap cmap{};
	for (size_t i = 0; i < NCHAR; ++i) cmap[i] = v;
	return cmap;
}

template <size_t L>
struct CharMaps : std::array<CharMap, L>
{};

template <size_t L>
struct Pattern {
	CharMaps<L> cmaps;
	const char* name;

	std::string tokenize(const char*& ch) const {
		std::cout << "tokenize: " << name << "\n";
		std::string token = "";
		int i_cm = 0;
		while (*ch) {
			int next_i_cm = cmaps[i_cm][*ch];
			if (next_i_cm == CMAP_LAST) {
				token += *ch++;
				return token;
			}
			else if (next_i_cm >= 0) {
				token += *ch++;
				i_cm = next_i_cm;
			}
			else {
				return token;
			}
		}
		return token;
	}
};

template <size_t L>
constexpr Pattern<1> _pattern(std::array<uchar, L> chars) {
	CharMap cm = _charmap(CMAP_NOT);
	for (char ch : chars) cm[ch] = CMAP_LAST;
	return { {cm} };
}

template <typename ...Args>
constexpr Pattern<1> chars(Args... args) {
	return _pattern(_chars(args...));
}

constexpr Pattern<1> char_(uchar ch) {
	return _pattern(_char(ch));
}

template <uchar CB, uchar CE>
constexpr Pattern<1> chars_range() {
	return _pattern(_chars_range<CB, CE>());
}


template <size_t L1, size_t L2>
constexpr CharMaps<L2> _shift(CharMaps<L2> cmaps) {
	CharMaps<L2> new_cmaps = cmaps;
	for (CharMap& cm : new_cmaps) {
		for (size_t i = 0; i < NCHAR; ++i) {
			if (cm[i] >= 0) cm[i] += L1;
		}
	}
	return new_cmaps;
}

template <size_t L1, size_t L2, size_t... Ls>
constexpr auto concat(Pattern<L1> pat1, Pattern<L2> pat2, Pattern<Ls>... pats) {
	return concat(concat(pat1, pat2), pats...);
}

template <size_t L1, size_t L2>
constexpr Pattern<L1 + L2> concat(/* copy */ Pattern<L1> cms1, /* copy */ Pattern<L2> cms2) {
	for (CharMap& cm : cms1.cmaps) {
		for (size_t i = 0; i < NCHAR; ++i) {
			if (cm[i] == CMAP_LAST) {
				cm[i] = L1;	//	First cmap of cms2
			}
			else if (cm[i] == CMAP_END) {
				cm[i] = cms2.cmaps[0][i];	//	Dest of first cmap of cms2
			}
		}
	}
	return { _concat_array(cms1.cmaps, _shift<L1>(cms2.cmaps)) };
}

template <size_t L>
constexpr auto _concat(Pattern<L> pat) {
	return pat;
}

template <size_t L>
constexpr Pattern<L> repeat(/* copy */ Pattern<L> pat) {
	for (CharMap& cm : pat.cmaps) {
		for (size_t i = 0; i < NCHAR; ++i) {
			if (cm[i] == CMAP_LAST) {
				cm[i] = 0;
			}
			else if (cm[i] == CMAP_END) {
				cm[i] = pat.cmaps[0][i];
			}
		}
	}
	return pat;
}

template <size_t L>
constexpr Pattern<L> opt(/* copy */ Pattern<L> pat) {
	CharMap& cm = pat.cmaps[0];
	for (size_t i = 0; i < NCHAR; ++i) {
		if (cm[i] == CMAP_NOT) {
			cm[i] = CMAP_END;
		}
	}
	return pat;
}

template <size_t L1, size_t L2, size_t... Ls>
constexpr auto or_(Pattern<L1> pat1, Pattern<L2> pat2, Pattern<Ls>... pats) {
	return or_(or_(pat1, pat2), pats...);
}

template <size_t L1, size_t L2>
constexpr Pattern<L1 + L2> or_(/* copy */ Pattern<L1> pat1, /* copy */ Pattern<L2> pat2) {
	CharMap& p1cm0 = pat1.cmaps[0];
	Pattern<L2> pat2new = { _shift<L1>(pat2.cmaps) };
	const CharMap& p2cm0 = pat2new.cmaps[0];
	for (size_t i = 0; i < NCHAR; ++i) {
		if ((p1cm0[i] <= CMAP_NOT && p2cm0[i] >= CMAP_END)
			|| (p1cm0[i] <= CMAP_END && p2cm0[i] >= CMAP_LAST)) {
			p1cm0[i] = p2cm0[i];
		}
	}
	return { _concat_array(pat1.cmaps, pat2new.cmaps) };
}

template <uchar CH>
constexpr Pattern<1> not_char() {
	return or_(chars_range<0, CH - 1>(), chars_range<CH + 1, NCHAR - 1>());
}

template <size_t L>
constexpr Pattern<L> named(const char* name, /* copy */ Pattern<L> pat) {
	return { pat.cmaps, name };
}


template <size_t L0, size_t... Ls>
struct Patterns {
	Pattern<L0> pat0;
	Patterns<Ls...> pats;
	constexpr Patterns(Pattern<L0> pat0, Pattern<Ls>... pats)
		: pat0(pat0), pats(pats...) {}

	std::string tokenize(int i_pat, const char*& ch) const {
		if (!i_pat) return pat0.tokenize(ch);
		return pats.tokenize(i_pat - 1, ch);
	}
};

template <size_t L0>
struct Patterns<L0> {
	Pattern<L0> pat0;
	constexpr Patterns(Pattern<L0> pat0)
		: pat0(pat0) {}

	std::string tokenize(int i_pat, const char*& ch) const {
		if (!i_pat) return pat0.tokenize(ch);
		throw std::exception("Index out of range.");
	}
};

template <size_t... Ls>
struct PatternMap : Patterns<Ls...> {
	CharMap cm0 = _charmap(CMAP_NOT);
	constexpr PatternMap(const CharMap& cm0, const Pattern<Ls>&... pats)
		: cm0(cm0), Patterns<Ls...>(pats...) {}

	std::vector<std::string> tokenize(const char* ch) const {
		std::vector<std::string> tokens;
		while (*ch) {
			int i_pat = cm0[*ch];
			if (i_pat >= 0) {
				std::string token = Patterns<Ls...>::tokenize(i_pat, ch);
				if (token.length()) tokens.push_back(token);
			}
			else {
				ch++;
			}
		}
		return tokens;
	}
};

template <size_t... Ls>
constexpr PatternMap<Ls...> make_pattern_map(const Pattern<Ls>&... pats) {
	CharMap cm0 = _charmap(CMAP_NOT);
	_make_pattern_map(cm0, 0, pats...);
	return PatternMap<Ls...>(cm0, pats...);
}

constexpr void _make_pattern_map(CharMap& cm0, size_t index) {}

template <size_t L1, size_t... Ls>
constexpr void _make_pattern_map(CharMap& cm0, size_t index, const Pattern<L1>& pat1, const Pattern<Ls>&... pats) {
	const CharMap& pat1_cm0 = pat1.cmaps[0];
	for (size_t i = 0; i < NCHAR; ++i) {
		if ((cm0[i] <= CMAP_NOT && pat1_cm0[i] >= CMAP_END)
			|| (cm0[i] <= CMAP_END && pat1_cm0[i] >= CMAP_LAST)) {
			cm0[i] = index;
		}
	}
	return _make_pattern_map(cm0, index + 1, pats...);
}
