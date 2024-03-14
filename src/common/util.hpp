#pragma once

#include <string>
#include <variant>

namespace shl
{
    template <typename... new_types, typename... old_types>
    std::variant<new_types...> cast_variant(const std::variant<old_types...>& old_variant)
    {
        return std::visit
        (
            []<typename type> requires(std::disjunction_v<std::is_same<std::decay_t<type>, new_types>...>) (type&& t) -> std::variant<new_types...>
            {
                return t;
            },
            old_variant
        );
    }

	template <typename T, typename... Ts>
	constexpr bool is_any_of_v = (std::is_same_v<T, Ts> || ...);

    // Return a new string with the contents of string, while having after inserted after each character in string.
    std::string insert_after_each(std::string_view string, std::string_view after);
} // namespace shl
