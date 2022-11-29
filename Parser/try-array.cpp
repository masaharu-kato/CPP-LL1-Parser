#include <iostream>
#include <algorithm>
#include <array>
#include <tuple>


template <typename T, size_t L1, size_t... I1, size_t L2, size_t... I2>
std::array<T, L1 + L2> _concat(std::array<T, L1> a1, std::array<T, L2> a2, std::index_sequence<I1...>, std::index_sequence<I2...>) {
	return {a1[I1]..., a2[I2]...};
}

template <typename T, size_t L1, size_t L2>
std::array<T, L1 + L2> concat(std::array<T, L1> a1, std::array<T, L2> a2) {
	return _concat<T>(a1, a2, std::make_index_sequence<L1>{}, std::make_index_sequence<L2>{});
}

template <size_t L, typename T, size_t... I>
std::array<T, L> _init_array(T v, std::index_sequence<I...>) {
	return {(I, v)...};
}

template <size_t L, typename T>
std::array<T, L> init_array(T v) {
	return _init_array<L>(v, std::make_index_sequence<L>{});
}

template<typename T, size_t L, size_t... I>
std::array<std::tuple<size_t, T>, L> _enumerate(const std::array<T, L>& a, std::index_sequence<I...>) {
	return {std::make_tuple(I, a[I])...};
}

template<typename T, size_t L>
std::array<std::tuple<size_t, T>, L> enumerate(const std::array<T, L>& a) {
	return _enumerate(a, std::make_index_sequence<L>{});
}

std::array<const char*, 5> arr1 = {"hoge", "fugar", "piyopiyo", "hey", "yeah"};
std::array<const char*, 3> arr2 = {"hello", "my", "c++"};
//constexpr std::array<int, 3> arr1 = {1,2,3};
//constexpr std::array<int, 2> arr2 = {4,5};
auto arr12 = concat(arr1, arr2);
auto arr3 = init_array<4>("unknown");

int main(void) {
	std::cout << "arr12" << std::endl;
	for(const char* str: arr12) std::cout << str << std::endl;
	std::cout << "arr12 (with index)" << std::endl;
	for(auto&& [i, str]: enumerate(arr12)) std::cout << i << ": " << str << std::endl;
	std::cout << "arr12c" << std::endl;
	for(const char* str: arr3) std::cout << str << std::endl;
	//for(int v: arr12) std::cout << v << std::endl;
	return 0;
}
