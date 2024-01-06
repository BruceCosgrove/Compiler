#pragma once

#include <array>
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
        _open_parenthesis,  // (
        _close_parenthesis, // )
        _open_brace,        // {
        _close_brace,       // }
        _open_bracket,      // [
        _close_bracket,     // ]
        _identifier,
        _let,
        _equals,
        _if,

        // Purely binary operators.
        _forward_slash, // division
        _percent,       // modulo

        // Both binary and unary operators.
        _asterisk, // binary: multiplication, unary: indirection/dereference
        _plus,     // binary: addition,       unary: promotion
        _minus,    // binary: subtraction,    unary: negation

        // Unary operators
        // _tilde, // bitwise not

        // Internals.
        count,

        binary_operators_begin = _forward_slash,
        binary_operators_end = _minus + 1,

        unary_operators_begin = _asterisk,
        unary_operators_end = _minus + 1,

        binary_and_unary_operators_begin = unary_operators_begin,
        binary_and_unary_operators_end = binary_operators_end,

        operators_begin = binary_operators_begin,
        operators_end = unary_operators_end,
    };

    [[nodiscard]] constexpr auto operator+(const token_type type) noexcept
    { return static_cast<std::underlying_type_t<token_type>>(type); }

    [[nodiscard]] constexpr bool is_binary_operator(const token_type type) noexcept
    { return +token_type::binary_operators_begin <= +type && +type < +token_type::binary_operators_end; }

    [[nodiscard]] constexpr bool is_unary_operator(const token_type type) noexcept
    { return +token_type::unary_operators_begin <= +type && +type < +token_type::unary_operators_end; }

    [[nodiscard]] constexpr bool is_binary_and_unary_operator(const token_type type) noexcept
    { return +token_type::binary_and_unary_operators_begin <= +type && +type < +token_type::binary_and_unary_operators_end; }

    [[nodiscard]] constexpr bool is_operator(const token_type type) noexcept
    { return +token_type::operators_begin <= +type && +type < +token_type::operators_end; }

    [[nodiscard]] constexpr std::optional<std::uint8_t> get_operator_precedence(const token_type type) noexcept
    {
        if (is_operator(type))
        {
            constexpr auto precedences = std::to_array<std::uint8_t>
            ({
                1, // token_type::_forward_slash
                1, // token_type::_percent

                1, // token_type::_asterisk
                0, // token_type::_plus
                0, // token_type::_minus
            });
            return precedences[+type - +token_type::operators_begin];
        }
        return std::nullopt;
    }

    struct token
    {
        token_type type;
        std::optional<std::string_view> value;
    };
} // namespace shl
