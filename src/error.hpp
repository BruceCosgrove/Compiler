#pragma once

#include <cstdint>
#include <string_view>

namespace shl
{
    [[noreturn]] void error_exit(std::string_view stage, std::string_view error_message, std::uint32_t line_number = 0, std::uint32_t column_number = 0);
} // namespace shl

#define ERROR_EXIT(stage, error_stream_args, ...) \
    do { \
        std::cerr << '[' << (stage) << " error]: " << error_stream_args << '.' << std::endl; \
        exit(EXIT_FAILURE); \
    } while (false)
