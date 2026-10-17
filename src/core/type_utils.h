#pragma once

#include <variant>
#include <type_traits>

template<typename T>
struct is_variant : std::false_type {};

template<typename... Ts>
struct is_variant<std::variant<Ts...>> : std::true_type {};

template<typename T>
constexpr bool is_variant_v = is_variant<T>::value;

template<typename Variant, typename T>
struct variant_contains;

template<typename T, typename... Ts>
struct variant_contains<std::variant<Ts...>, T>
	: std::disjunction<std::is_same<T, Ts>...> {
};

template<typename Variant, typename T>
constexpr bool variant_contains_v = variant_contains<Variant, T>::value;

template<typename Target, typename T>
Target* valuePtr(T& value) {
	if constexpr (std::is_same_v<T, Target>) {
		return &value;
	}
	else if constexpr (is_variant_v<T> && variant_contains_v<T, Target>) {
		return std::get_if<Target>(&value);
	}
	else {
		return nullptr;
	}
}
