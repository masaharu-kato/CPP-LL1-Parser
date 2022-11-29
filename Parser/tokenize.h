#pragma once
#include <array>
#include <stack>
#include <pair>
#include <tuple>
#include <string>
#include <algorithm>
#include <iostream>
#include <vector>
#include "cexpr_array_utils.h"

constexpr size_t NCHAR = 256;
using uchar = unsigned char;


struct CharState {
	constexpr static int ERROR_END = -1;

	bool valid = false;	//	Is a valid state (if false, quit tokenize)
	int next_cm = 0;	//	Relative index to the next charmap
	int out_before = 0;	//	Output until the previous character
	int out_after = 0;  //	Output until the current character
	int stack = 0;		//	Stack update (+1: push, 0: none, -1: pop)
};

//using _CharMap = std::array<CharState, NCHAR>;
//constexpr int CMAP_LAST = -1;	//	The token will ends on the current positon
//constexpr int CMAP_END = -2;	//	The token ended on the previous position
//constexpr int CMAP_NOT = -3;	//	Invalid character for this token

//constexpr _CharMap _init_charmap(int v) {
//	_CharMap cmap{};
//	for (size_t i = 0; i < NCHAR; ++i) cmap[i] = v;
//	return cmap;
//}


//struct CharMap : _CharMap {
//
//	//template <typename... Args>
//	//constexpr CharMap(Args... args) : _CharMap{args...} {}
//
//	//constexpr CharMap() : _CharMap() {}
//	//constexpr CharMap(_CharMap chmap) : _CharMap(chmap) {}
//	//constexpr CharMap(uchar ch) : _CharMap(_charmap(ch)) {}
//	//template <typename... Args>
//	//constexpr CharMap(uchar ch, Args... chs) : _CharMap(_charmap(ch, chs...)) {}
//};


//template <size_t L>
//constexpr _CharMap _charmap(std::array<uchar, L> chars) {
//	_CharMap cm0{}, cm1{};
//	for (uchar ch : chars) cm0[ch] = 1;
//	return cm;
//}

//template <typename... Args>
//constexpr _CharMap _charmap(Args... chs) {
//	return _charmap(std::array<uchar, sizeof...(Args)>{(uchar)chs...});
//}

struct StringView {
	const char* chbeg;
	const char* chend;

	operator std::string() const {
		return std::string(chbeg, chend);
	}

	size_t size() const { return chend - chbeg; }
	size_t length() const { return size(); }
};

struct Rule {
	int id;
};

struct _Token {
	int i_cm;
	StringView str;
};

struct Token {
	Rule rule;
	StringView str;
};

struct TokenChildren;

struct TokenWithChild {
	Token token;
	TokenChildren* children;
};

struct TokenChildren : std::vector<TokenWithChild> {};


using CharMap = std::array<CharState, NCHAR>;

CharMap init_charmap(CharState cs) {
	return init_array<NCHAR>(cs);
}

template <size_t L>
using CharMaps = std::array<CharMap, L>;

template <size_t L>
struct Pattern : CharMaps<L>
{

	TokenChildren tokenize(const char*& ch) const {
		const char* chbeg = ch;
		int i_cm = 0;
		std::stack<TokenChildren*> stk{};
		while (*ch) {
			CharState cs = (*this)[i_cm][*ch];
			if (cs.stack == 1) stk.push({});
			if (cs.stack == -1) { children = stk.top(); stk.pop(); }
			if (cs.out_before) {
				stk.top().emplace_back(cs.out_token, { {chbeg, ch}, children });
				chbeg = ch;
				children = {};
			}
			if (!cs.valid) break;
			ch++;
			if (cs.out_before) {
				stk.top().emplace_back(cs.out_token, { {chbeg, ch}, children });
				chbeg = ch;
				children = {};
			}
			i_cm += cs.next_cm;
		}
		return stk.top();
	}

	//	Concat
	template <size_t L2>
	constexpr Pattern<L + L2> _concat(const Pattern<L2>& pat2) const {
		//	Concat the End states of *this and First states of pat2
		Pattern pat1 = *this;
		for (Enumed<CharMap&>[i_cm, cm] : enumerate(pat1)) {
			for (CharState& cs : cm) {
				if (!cs.valid && cs.out_token) {
					cs.valid = true;
					cs.next_cm = L - i_cm;	//	CharMap #0 in `pat2`
				}
			}
		}
		return array_concat(pat1, pat2);
	}

	//	Concat
	template <size_t L2>
	constexpr Pattern<L + L2> operator ,(const Pattern<L2>& pat2) const {
		Pattern<L + L2> pat12 = _concat(pat2);

		//	Set stack push/pop
		for (CharState& cs : pat12[0]) if (cs.valid) cs.stack++;
		for (CharMap& cm : pat12) {
			for (CharState& cs : cm) {
				if (!cs.valid && cs.out_token) cs.stack--;
			}
		}

		return { pat12 };
	}

	//	Or
	template <size_t L2>
	constexpr Pattern<L + L2> operator |(const Pattern<L2>& pat2) const {
		Pattern pat1 = *this;
		CharMap& cm10 = pat1[0];
		const CharMap& cm20 = pat2[L];
		for (size_t ch = 0; ch < NCHAR; ++ch) {
			if (cm10[ch].end_id == CharState::ERROR_END) cm10[ch] = cm20[ch];
		}
		return { array_concat(pat1, pat2) };
	}

	//	Repeat (1+)
	constexpr Pattern<L + 1> operator +() const {
		Pattern pat1 = *this;
		for (CharMap& cm : pat1) {
			for (size_t ch = 0; ch < NCHAR; ++ch) {
				if (cm[i] == CMAP_LAST) {
					cm[i] = 0;
				}
				else if (cm[i] == CMAP_END) {
					cm[i] = pat1[0][i];
				}
			}
		}
		return pat1;
	}

	//	Optional
	constexpr Pattern<L> operator ~() {
		auto new_endrules = endrules;
		new_endrules[0] = 1;
		return { chmaps, {} }
	}

	//	Repeat (0+)
	constexpr Pattern<L> operator *() const {
		Pattern pat1 = *this;
		for (CharMap& cm : pat1) {
			for (size_t ch = 0; ch < NCHAR; ++ch) {
				if (cm[i] == CMAP_LAST) {
					cm[i] = 0;
				}
				else if (cm[i] == CMAP_END) {
					cm[i] = pat1[0][i];
				}
			}
		}
		return pat1;
	}

};

constexpr Pattern<2> pattern(uchar ch) {
	CharMap cm0{};
	cm0[ch] = { true, 1 };
	CharMap cm1 = init_charmap({ false, 0, 1 });
	return { {cm0, cm1} };
}

template <size_t L>
constexpr Pattern<2> pattern(std::array<uchar, L> chs) {
	CharMap cm0{};
	for (uchar ch : chs) cm0[ch] = { true, 1 };
	CharMap cm1 = init_charmap({ false, 0, 1 });
	return { {cm0, cm1} };
}

constexpr Pattern<2> char_range(uchar chbeg, uchar chend) {
	CharMap cm0{};
	for (size_t ch = chbeg; ch <= chend; ++ch) cm0[ch] = { true, 1 };
	CharMap cm1 = init_charmap({ false, 0, 1 });
	return { {cm0, cm1} };
}


using PatternName = const char*;
constexpr PatternName NullPatternName = nullptr;

template <size_t L>
struct NamedPattern : Pattern<L> {
	PatternName name;

	constexpr NamedPattern(Pattern<L> pat, PatternName name = NullPatternName) : Pattern<L>(pat), name(name) {}

	//constexpr NamedPattern(CharMap cmap) : NamedPattern(Pattern<L>(cmap)) {}
	//constexpr NamedPattern(_CharMap _cmap) : NamedPattern(Pattern<L>(CharMap(_cmap))) {}
	//constexpr NamedPattern(uchar ch) : NamedPattern(Pattern<L>(CharMap(ch))) {}
	//template <typename... Args>
	//constexpr NamedPattern(Args... args) : NamedPattern(Pattern<L>(CharMap(args...))) {}

	std::string tokenize(const char*& ch) const {
		std::cout << "tokenize: " << name << "\n";
		return Pattern<L>::tokenize(ch);
	}

	//template <size_t L2>
	//constexpr NamedPattern<L + L2> operator ,(NamedPattern<L2> pat) const {
	//	return {(cmaps, pat.cmaps)};
	//}

	//template <typename _T>
	//constexpr NamedPattern<L + 1> operator ,(_T patlike) const {
	//	return operator ,(NamedPattern<1>(patlike));
	//}

	//template <size_t L2>
	//constexpr NamedPattern<L + L2> operator |(NamedPattern<L2> pat) const {
	//	return { cmaps | pat.cmaps };
	//}

	//template <typename _T>
	//constexpr NamedPattern<L + 1> operator |(_T patlike) const {
	//	return operator |(NamedPattern<1>(patlike));
	//}

	////	Repeat (1+)
	//constexpr NamedPattern<L> operator +() const {
	//	return { +cmaps };
	//}

	////	Optional
	//constexpr NamedPattern<L> operator ~() {
	//	return { ~cmaps };
	//}

	////	Repeat (0+)
	//constexpr NamedPattern<L> operator *() const {
	//	return { *cmaps };
	//}


};

//template <typename _T, size_t L>
//constexpr NamedPattern<L + 1> operator ,(_T patlike, NamedPattern<L> pat) {
//	return NamedPattern<1>(patlike) , pat;
//}

//template <typename _T, size_t L>
//constexpr NamedPattern<L + 1> operator |(_T patlike, NamedPattern<L> pat) {
//	return NamedPattern<1>(patlike) | pat;
//}

template <size_t L>
constexpr NamedPattern<L> named(const char* name, /* copy */ NamedPattern<L> pat) {
	return { pat.cmaps, name };
}


template <size_t L0, size_t... Ls>
struct NamedPatterns {
	NamedPattern<L0> pat0;
	NamedPatterns<Ls...> pats;
	constexpr NamedPatterns(NamedPattern<L0> pat0, NamedPattern<Ls>... pats)
		: pat0(pat0), pats(pats...) {}

	std::string tokenize(int i_pat, const char*& ch) const {
		if (!i_pat) return pat0.tokenize(ch);
		return pats.tokenize(i_pat - 1, ch);
	}
};

template <size_t L0>
struct NamedPatterns<L0> {
	NamedPattern<L0> pat0;
	constexpr NamedPatterns(NamedPattern<L0> pat0)
		: pat0(pat0) {}

	std::string tokenize(int i_pat, const char*& ch) const {
		if (!i_pat) return pat0.tokenize(ch);
		throw std::runtime_error("Index out of range.");
	}
};

template <size_t... Ls>
struct PatternMap : NamedPatterns<Ls...> {
	CharMap cm0{};
	constexpr PatternMap(const CharMap& cm0, const NamedPattern<Ls>&... pats)
		: cm0(cm0), NamedPatterns<Ls...>(pats...) {}

	std::vector<std::string> tokenize(const char* ch) const {
		std::vector<std::string> tokens;
		while (*ch) {
			int i_pat = cm0[*ch];
			if (i_pat >= 0) {
				std::string token = NamedPatterns<Ls...>::tokenize(i_pat, ch);
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
constexpr PatternMap<Ls...> make_pattern_map(const NamedPattern<Ls>&... pats) {
	CharMap cm0{};
	_make_pattern_map(cm0, 0, pats...);
	return PatternMap<Ls...>(cm0, pats...);
}

constexpr void _make_pattern_map(CharMap& cm0, size_t index) {}

template <size_t L1, size_t... Ls>
constexpr void _make_pattern_map(CharMap& cm0, size_t index, const NamedPattern<L1>& pat1, const NamedPattern<Ls>&... pats) {
	const CharMap& pat1_cm0 = pat1.chmaps[0];
	for (size_t i = 0; i < NCHAR; ++i) {
		if ((cm0[i] <= CMAP_NOT && pat1_cm0[i] >= CMAP_END)
			|| (cm0[i] <= CMAP_END && pat1_cm0[i] >= CMAP_LAST)) {
			cm0[i] = index;
		}
	}
	return _make_pattern_map(cm0, index + 1, pats...);
}
