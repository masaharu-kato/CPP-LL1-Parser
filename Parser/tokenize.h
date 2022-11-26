#pragma once
#include <array>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

constexpr size_t NCHAR = 256;
using uchar = unsigned char;

using _CharMap = std::array<int, NCHAR>;
constexpr int CMAP_LAST = -1;	//	The token will ends on the current positon
constexpr int CMAP_END = -2;	//	The token ended on the previous position
constexpr int CMAP_NOT = -3;	//	Invalid character for this token

constexpr _CharMap _init_charmap(int v) {
	_CharMap cmap{};
	for (size_t i = 0; i < NCHAR; ++i) cmap[i] = v;
	return cmap;
}

template <size_t L>
constexpr _CharMap _charmap(std::array<uchar, L> chars) {
	_CharMap cm = _init_charmap(CMAP_NOT);
	for (char ch : chars) cm[ch] = CMAP_LAST;
	return cm;
}

template <typename... Args>
constexpr _CharMap _charmap(Args... chs) {
	return _charmap(std::array<uchar, sizeof...(Args)>{(uchar)chs...});
}

struct CharMap : _CharMap {

	constexpr CharMap() : _CharMap() {}
	constexpr CharMap(_CharMap chmap) : _CharMap(chmap) {}
	constexpr CharMap(uchar ch) : _CharMap(_charmap(ch)) {}
	template <typename... Args> 
	constexpr CharMap(uchar ch, Args... chs) : _CharMap(_charmap(ch, chs...)) {}

	static constexpr CharMap range(uchar chbeg, uchar chend) {
		_CharMap cm = _init_charmap(CMAP_NOT);
		for (size_t ch=chbeg; ch<=chend; ++ch) cm[ch] = CMAP_LAST;
		return CharMap(cm);
	}
};

template <size_t L>
struct CharMaps : std::array<CharMap, L>
{
	template <typename... Args>
	constexpr CharMaps(Args... args) : std::array<CharMap, L>{args...} {}

	template <size_t _L>
	constexpr CharMaps<L+_L> operator +(CharMaps<_L> cms) const {
		CharMaps<L+_L> res{};
		for(size_t i=0; i<L; ++i) res[i] = (*this)[i];
		for(size_t i=0; i<_L; ++i) res[i+L] = cms[i];
		return res;
	}
};

using PatternName = const char*;
constexpr PatternName NullPatternName = nullptr;

template <size_t L>
struct Pattern {
	CharMaps<L> cmaps;
	PatternName name;

	constexpr Pattern(CharMaps<L> cmaps, PatternName name = NullPatternName) : cmaps(cmaps), name(name) {}

	//constexpr Pattern(CharMap cmap) : Pattern(CharMaps<L>(cmap)) {}
	//constexpr Pattern(_CharMap _cmap) : Pattern(CharMaps<L>(CharMap(_cmap))) {}
	//constexpr Pattern(uchar ch) : Pattern(CharMaps<L>(CharMap(ch))) {}
	template <typename... Args>
	constexpr Pattern(Args... args) : Pattern(CharMaps<L>(CharMap(args...))) {}

	static constexpr Pattern<1> range(uchar chbeg, uchar chend) {
		return Pattern<1>(CharMap::range(chbeg, chend));
	}

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

	//	Concat
	template <size_t _L>
	constexpr Pattern<L + _L> operator ,(const Pattern<_L>& _pat) const {
		CharMaps<L> new_cmaps = cmaps;
		Pattern<_L> pat = _pat.template _shift<L>();
		for (CharMap& cm : new_cmaps) {
			for (size_t i = 0; i < NCHAR; ++i) {
				if (cm[i] == CMAP_LAST) {
					cm[i] = L;	//	First cmap of pat
				}
				else if (cm[i] == CMAP_END) {
					cm[i] = pat.cmaps[0][i];	//	Dest of first cmap of pat
				}
			}
		}
		return { new_cmaps + pat.cmaps };
	}

	template <typename _T>
	constexpr Pattern<L + 1> operator ,(_T patlike) const {
		return operator ,(Pattern<1>(patlike));
	}

	//	Or
	template <size_t _L>
	constexpr Pattern<L + _L> operator |(const Pattern<_L>& _pat) const {
		CharMaps<L> new_cmaps = cmaps;
		CharMap& cm0 = new_cmaps[0];
		Pattern<_L> pat = _pat.template _shift<L>();
		const CharMap& pcm0 = pat.cmaps[0];
		for (size_t i = 0; i < NCHAR; ++i) {
			if ((cm0[i] <= CMAP_NOT && pcm0[i] >= CMAP_END)
				|| (cm0[i] <= CMAP_END && pcm0[i] >= CMAP_LAST)) {
				cm0[i] = pcm0[i];
			}
		}
		return { new_cmaps + pat.cmaps };
	}

	template <typename _T>
	constexpr Pattern<L + 1> operator |(_T patlike) const {
		return operator |(Pattern<1>(patlike));
	}

	//	Repeat (1+)
	constexpr Pattern<L> operator +() const {
		Pattern<L> pat = *this;
		for (CharMap& cm : pat.cmaps) {
			for (size_t i = 0; i < NCHAR; ++i) {
				if (cm[i] == CMAP_LAST) {
					cm[i] = 0;
				}
				else if (cm[i] == CMAP_END) {
					cm[i] = cmaps[0][i];
				}
			}
		}
		return pat;
	}

	//	Optional
	constexpr Pattern<L> operator ~() {
		Pattern<L> pat = *this;
		CharMap& cm = pat.cmaps[0];
		for (size_t i = 0; i < NCHAR; ++i) {
			if (cm[i] == CMAP_NOT) {
				cm[i] = CMAP_END;
			}
		}
		return pat;
	}

	//	Repeat (0+)
	constexpr Pattern<L> operator *() const {
		return ~+*this;
	}

	template <size_t _L>
	constexpr Pattern<L> _shift() const {
		Pattern<L> pat = *this;
		for (CharMap& cm : pat.cmaps) {
			for (size_t i = 0; i < NCHAR; ++i) {
				if (cm[i] >= 0) cm[i] += _L;
			}
		}
		return pat;
	}

};

//template <typename _T, size_t L>
//constexpr Pattern<L + 1> operator ,(_T patlike, Pattern<L> pat) {
//	return Pattern<1>(patlike) , pat;
//}

//template <typename _T, size_t L>
//constexpr Pattern<L + 1> operator |(_T patlike, Pattern<L> pat) {
//	return Pattern<1>(patlike) | pat;
//}

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
		throw std::runtime_error("Index out of range.");
	}
};

template <size_t... Ls>
struct PatternMap : Patterns<Ls...> {
	CharMap cm0 = _init_charmap(CMAP_NOT);
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
	CharMap cm0 = _init_charmap(CMAP_NOT);
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
