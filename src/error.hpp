#pragma once

#include <string_view>

namespace shl
{
    [[noreturn]] void error_exit(const std::string_view error_message);
} // namespace shl
