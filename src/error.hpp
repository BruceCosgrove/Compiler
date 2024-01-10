#pragma once

#include <cstdint>
#include <string_view>

namespace shl
{
    [[noreturn]] void error_exit(std::string_view stage, std::string_view error_message, std::uint32_t line_number = 0, std::uint32_t column_number = 0);
} // namespace shl
