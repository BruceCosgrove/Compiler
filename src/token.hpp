#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

namespace shl
{
    enum class token_type
    {
        // Correspondence: Literal characters in grammar rules.

        open_parenthesis_,  // (
        close_parenthesis_, // )
        open_brace_,        // {
        close_brace_,       // }
        open_bracket_,      // [
        close_bracket_,     // ]
        colon_,             // :
        semicolon_,         // ;
        comma_,             // ,
        equals_,            // =

        // Correspondence: Trivial grammar rule.

        integer_literal_,
        identifier_,

        // Correspondence: Non-trivial grammar rule case.

        let_,
        return_,
        if_,
        elif_,
        else_,

        // Correspondence: Binary-only operators.

        forward_slash_, // binary: division
        percent_,       // binary: modulo

        // Correspondence: Binary and unary operators.

        asterisk_, // binary: multiplication, unary: indirection/dereference
        plus_,     // binary: addition,       unary: promotion
        minus_,    // binary: subtraction,    unary: negation

        // Correspondence: Unary-only operators

        // _tilde, // unary: bitwise not

        // Correspondence: Parameter passing.

        in_,
        out_,
        inout_,
        copy_,
        move_,

        // Internals.
        _count,

        _binary_operators_begin = forward_slash_,
        _binary_operators_end = minus_ + 1,

        _unary_operators_begin = asterisk_,
        _unary_operators_end = minus_ + 1,

        _binary_and_unary_operators_begin = _unary_operators_begin,
        _binary_and_unary_operators_end = _binary_operators_end,

        _operators_begin = _binary_operators_begin,
        _operators_end = _unary_operators_end,
    };

    [[nodiscard]] constexpr auto operator+(const token_type type) noexcept
    { return static_cast<std::underlying_type_t<token_type>>(type); }

    [[nodiscard]] constexpr bool is_binary_operator(const token_type type) noexcept
    { return +token_type::_binary_operators_begin <= +type && +type < +token_type::_binary_operators_end; }

    [[nodiscard]] constexpr bool is_unary_operator(const token_type type) noexcept
    { return +token_type::_unary_operators_begin <= +type && +type < +token_type::_unary_operators_end; }

    [[nodiscard]] constexpr bool is_binary_and_unary_operator(const token_type type) noexcept
    { return +token_type::_binary_and_unary_operators_begin <= +type && +type < +token_type::_binary_and_unary_operators_end; }

    [[nodiscard]] constexpr bool is_operator(const token_type type) noexcept
    { return +token_type::_operators_begin <= +type && +type < +token_type::_operators_end; }

    [[nodiscard]] constexpr std::optional<std::uint8_t> get_operator_precedence(const token_type type) noexcept
    {
        if (is_operator(type))
        {
            constexpr auto precedences = std::to_array<std::uint8_t>
            ({
                1, // token_type::forward_slash_
                1, // token_type::percent_

                1, // token_type::asterisk_
                0, // token_type::plus_
                0, // token_type::minus_
            });
            return precedences[+type - +token_type::_operators_begin];
        }
        return std::nullopt;
    }

    struct token
    {
        token_type type;
        std::optional<std::string_view> value;
    };
} // namespace shl
