#pragma once
#include <algorithm>
#include <array>
#include <tuple>

template <size_t L0, size_t... Ls>
size_t _sum() { return L0 + _sum<Ls...>(); }
size_t _sum() { return 0; }

template <typename T, size_t L1, size_t... I1, size_t L2, size_t... I2>
std::array<T, L1 + L2> _array_concat(std::array<T, L1> a1, std::array<T, L2> a2, std::index_sequence<I1...>, std::index_sequence<I2...>) {
	return {a1[I1]..., a2[I2]...};
}

template <typename T, size_t L1, size_t L2>
std::array<T, L1 + L2> array_concat(std::array<T, L1> a1, std::array<T, L2> a2) {
	return _array_concat<T>(a1, a2, std::make_index_sequence<L1>{}, std::make_index_sequence<L2>{});
}

template <size_t L, typename T, size_t... I>
std::array<T, L> _init_array(T v, std::index_sequence<I...>) {
	return {(I, v)...};
}

template <size_t L, typename T>
std::array<T, L> init_array(T v) {
	return _init_array<L>(v, std::make_index_sequence<L>{});
}

template <typename T>
using Enumed = std::tuple<size_t, T>;

template<typename T, size_t L, size_t... I>
std::array<Enumed<T>, L> _enumerate(const std::array<T, L>& a, std::index_sequence<I...>) {
	return {std::make_tuple(I, a[I])...};
}

template<typename T, size_t L>
std::array<Enumed<T>, L> enumerate(const std::array<T, L>& a) {
	return _enumerate(a, std::make_index_sequence<L>{});
}
