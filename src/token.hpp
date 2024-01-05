#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

namespace shl
{
    enum class token_type
    {
        _return,
        _integer_literal,
        _semicolon,
        _open_parenthesis,
        _close_parenthesis,
        _identifier,
        _let,
        _equals,
        _asterisk,
        _forward_slash,
        _percent,
        _plus,
        _minus,
    };

    constexpr auto operator+(const token_type type) noexcept
    {
        return static_cast<std::underlying_type_t<token_type>>(type);
    }

    struct token
    {
        token_type type;
        std::optional<std::string_view> value;
    };
} // namespace shl
