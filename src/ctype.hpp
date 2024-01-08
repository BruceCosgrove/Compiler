#pragma once

#include <cctype>

namespace shl
{
    inline bool is_space           (char c) noexcept { return std::isspace(c);             }
    inline bool is_digit_10        (char c) noexcept { return std::isdigit(c);             }
    inline bool is_identifier_first(char c) noexcept { return std::isalpha(c) || c == '_'; }
    inline bool is_identifier_rest (char c) noexcept { return std::isalnum(c) || c == '_'; }
    inline bool is_line_ending     (char c) noexcept { return c == '\n' || c == '\r';      }
    inline bool is_not_line_ending (char c) noexcept { return !is_line_ending(c);          }
} // namespace shl
