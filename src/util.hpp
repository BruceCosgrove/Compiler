#pragma once

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
} // namespace shl
