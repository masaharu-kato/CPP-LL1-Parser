#pragma once
#include <array>
#include <vector>
#include <string>
#include <iostream>

constexpr int NCHAR = 256;
using uchar = unsigned char;

template <typename T, typename ...Args>
constexpr auto _make_array(Args&&... args)
{
	return std::array<T, sizeof...(Args)>{args...};
}
template <typename ...Args>
constexpr auto _chars(Args&&... args)
{
	return std::array<uchar, sizeof...(Args)>{(uchar)args...};
}

template <uchar CB, uchar CE>
constexpr auto _chars_range() {
	std::array<uchar, CE-CB+1> chars{};
	for(uchar ch=CB; ch<=CE; ++ch) chars[ch-CB] = ch;
	return chars;
}

template <typename T, int L1, int L2>
constexpr std::array<T, L1+L2> _concat_array(std::array<T, L1> a1, std::array<T, L2> a2) {
	std::array<T, L1+L2> a12{};
	int i = 0;
	for(auto v: a1) a12[i++] = v;
	for(auto v: a2) a12[i++] = v;
	return a12;
}

//struct Pattern {
//	const char* name = nullptr;
//};

//template <int L>
//struct Single : Pattern {
//	const std::array<uchar, L> chars;
//	constexpr Single(const std::array<uchar, L> chars, const char* name = nullptr)
//		: chars(chars), Pattern{name} {}
//};

//struct CharMapPattern {
//	const char* name;
//	int next = 0;	//	Next CharMap Index
//};
using CharMap = std::array<int, NCHAR>;
constexpr int CMAP_LAST = -1;	//	The token will ends on the current positon
constexpr int CMAP_END = -2;	//	The token ended on the previous position
constexpr int CMAP_NOT = -3;	//	Invalid character for this token

constexpr CharMap _charmap(int v) {
	CharMap cmap{};
	for(int i=0; i<NCHAR; ++i) cmap[i] = v;
	return cmap;
}

template <int L>
struct Pattern {
	std::array<CharMap, L> cmap;
	const char* name;

	std::string tokenize(const char*& ch) const {
		std::cout << "tokenize: " << name << "\n";
		std::string token = "";
		int i_cm = 0;
		while (*ch) {
			int next_i_cm = cmap[i_cm][*ch];
			if (next_i_cm == CMAP_LAST) {
				token += *ch++;
				return token;
			} else if (next_i_cm >= 0) {
				token += *ch++;
				i_cm = next_i_cm;
			} else {
				return token;
			}
		}
		return token; 
	}
};

template <int L>
constexpr Pattern<1> one(std::array<uchar, L> chars) {
	CharMap cm = _charmap(CMAP_NOT);
	for (char ch : chars) cm[ch] = CMAP_LAST;
	return {{cm}};
}

template <int L>
constexpr Pattern<1> opt_single(std::array<uchar, L> chars) {
	CharMap cm = _charmap(CMAP_END);
	for (char ch : chars) cm[ch] = CMAP_LAST;
	return {{cm}};
}

//template <int L>
//struct Repeat : Pattern {
//	const std::array<uchar, L> chars;
//	constexpr Repeat(const std::array<uchar, L> chars)
//		: chars(chars), Pattern{name} {}
//};
template <int L>
constexpr Pattern<2> repeat(std::array<uchar, L> chars) {
	CharMap cm1 = _charmap(CMAP_NOT);
	for (char ch : chars) cm1[ch] = 1;
	CharMap cm2 = _charmap(CMAP_END);
	for (char ch : chars) cm2[ch] = 1;
	return {{cm1, cm2}};
}

template <int L>
constexpr Pattern<1> opt_repeat(std::array<uchar, L> chars) {
	CharMap cm = _charmap(CMAP_END);
	for (char ch : chars) cm[ch] = 0;
	return {{cm}};
}

template <int L1, int L2>
constexpr std::array<CharMap, L2> _shift(std::array<CharMap, L2> cmap) {
	std::array<CharMap, L2> new_cmap = cmap;
	for (CharMap& cm : new_cmap) {
		for(int i=0; i<NCHAR; ++i){
			if(cm[i] >= 0) cm[i] += L1;
		}
	}
	return new_cmap;
}

//template <typename... Ts>
//constexpr auto concat(Ts... vs) {
//	return _concat(to_paterns(vs...)...);
//}
//
//template <int L0, typename... Ts>
//constexpr auto to_patterns(Pattern<L0> v0, Ts... vs) {
//	return {v0, to_patterns(vs...)...};
//}
//
//template <int L0>
//constexpr auto to_patterns(Pattern<L0> v0) {
//	return {v0};
//}
//
//template <int L0, typename... Ts>
//constexpr auto to_patterns(std::array<uchar, L0> v0, Ts... vs) {
//	return {single(v0), to_patterns(vs...)...};
//}
//
//template <int L0>
//constexpr auto to_patterns(std::array<uchar, L0> v0) {
//	return {single(v0)};
//}

template <typename... Ts>
constexpr auto concat(Ts... vs) {
	//return _concat(to_paterns(vs...)...);
	return _concat(vs...);
}

template <int L1, int L2, int... Ls>
constexpr auto _concat(Pattern<L1> pat1, Pattern<L2> pat2, Pattern<Ls>... pats) {
	return _concat(_concat(pat1, pat2), pats...);
}

template <int L1, int L2>
constexpr Pattern<L1+L2> _concat(/* copy */ Pattern<L1> cms1, /* copy */ Pattern<L2> cms2) {
	for (CharMap& cm : cms1.cmap) {
		for(int i=0; i<NCHAR; ++i){
			if (cm[i] == CMAP_LAST) {
				cm[i] = L1;	//	First cmap of cms2
			}
			else if (cm[i] == CMAP_END) {
				cm[i] = cms2.cmap[0][i];	//	Dest of first cmap of cms2
			}
		}
	}
	return {_concat_array(cms1.cmap, _shift<L1>(cms2.cmap)), nullptr};
}

template <int L>
constexpr auto _concat(Pattern<L> pat) {
	return pat;
}

template <int L>
constexpr Pattern<L> repeat(/* copy */ Pattern<L> pat) {
	for (CharMap& cm: pat.cmap) {
		for (int i = 0; i < NCHAR; ++i) {
			if (cm[i] == CMAP_LAST) {
				cm[i] = 0;
			}
			else if (cm[i] == CMAP_END) {
				cm[i] = pat.cmap[0][i];
			}
		}
	}
	return pat;
}

template <int L>
constexpr Pattern<L> opt(/* copy */ Pattern<L> pat) {
	CharMap& cm = pat.cmap[0];
	for (int i = 0; i < NCHAR; ++i) {
		if (cm[i] == CMAP_NOT) {
			cm[i] = CMAP_END;
		}
	}
	return pat;
}

template <int L1, int L2>
constexpr Pattern<L1+L2> _or(/* copy */ Pattern<L1> pat1, /* copy */ Pattern<L2> pat2) {
	CharMap& p1cm0 = pat1.cmap[0];
	Pattern<L2> pat2new = {_shift<L1>(pat2.cmap)};
	const CharMap& p2cm0 = pat2new.cmap[0];
	for(int i=0; i<NCHAR; ++i){
		if (   (p1cm0[i] <= CMAP_NOT && p2cm0[i] >= CMAP_END)
			|| (p1cm0[i] <= CMAP_END && p2cm0[i] >= CMAP_LAST)) {
			p1cm0[i] = p2cm0[i];
		}
	}
	return {_concat_array(pat1.cmap, pat2new.cmap)};
}

template <int L>
constexpr Pattern<L> named(const char* name, /* copy */ Pattern<L> pat) {
	return {pat.cmap, name};
}


template <int L0, int... Ls>
struct Patterns {
	Pattern<L0> pat0;
	Patterns<Ls...> pats;
	constexpr Patterns(Pattern<L0> pat0, Pattern<Ls>... pats)
		: pat0(pat0), pats(pats...) {}

	std::string tokenize(int i_pat, const char*& ch) const {
		if(!i_pat) return pat0.tokenize(ch);
		return pats.tokenize(i_pat-1, ch);
	}
};

template <int L0>
struct Patterns<L0> {
	Pattern<L0> pat0;
	constexpr Patterns(Pattern<L0> pat0)
		: pat0(pat0) {}

	std::string tokenize(int i_pat, const char*& ch) const {
		if(!i_pat) return pat0.tokenize(ch);
		throw std::exception("Index out of range.");
	}
};

template <int... Ls>
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
				if(token.length()) tokens.push_back(token);
			}
			else {
				ch++;
			}
		}
		return tokens;
	}
};

template <int... Ls>
constexpr PatternMap<Ls...> make_pattern_map(const Pattern<Ls>&... pats) {
	CharMap cm0 = _charmap(CMAP_NOT);
	_make_pattern_map(cm0, 0, pats...);
	return PatternMap<Ls...>(cm0, pats...);
}

constexpr void _make_pattern_map(CharMap& cm0, int index) {}

template <int L1, int... Ls>
constexpr void _make_pattern_map(CharMap& cm0, int index, const Pattern<L1>& pat1, const Pattern<Ls>&... pats) {
	const CharMap& pat1_cm0 = pat1.cmap[0];
	for(int i=0; i<NCHAR; ++i){
		if (   (cm0[i] <= CMAP_NOT && pat1_cm0[i] >= CMAP_END)
			|| (cm0[i] <= CMAP_END && pat1_cm0[i] >= CMAP_LAST)) {
			cm0[i] = index;
		}
	}
	return _make_pattern_map(cm0, index+1, pats...);
}