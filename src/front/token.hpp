#pragma once

#include "common/ranged_enum.hpp"
#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

namespace shl
{
    DEFINE_RANGED_ENUM(token_type,
        (
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

            return_,
            if_,
            elif_,
            else_,
            while_,
            dowhile_,

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
            move_
        ),
        // Ranges
        (
            (binary_operator, forward_slash_, minus_ + 1),
            (unary_operator, asterisk_, minus_ + 1),
            (binary_and_unary_operator, _range_unary_operators_begin, _range_binary_operators_end),
            (operator, _range_binary_operators_begin, _range_unary_operators_end)
        )
    );

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
            return precedences[+type - +token_type::_range_operators_begin];
        }
        return std::nullopt;
    }

    struct token
    {
        token_type type;
        std::uint32_t line_number;
        std::uint32_t column_number;
        std::string_view value;
    };
} // namespace shl
