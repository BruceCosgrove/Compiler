#include "error.hpp"
#include <iostream>

namespace shl
{
    void error_exit(std::string_view stage, std::string_view error_message, std::uint32_t line_number, std::uint32_t column_number)
    {
        std::cerr << '[' << stage << " error]: " << error_message;
        if (line_number || column_number)
            std::cerr << " around " << line_number << ':' << column_number << " (+/- one token)";
        std::cerr << '.' << std::endl;
        exit(EXIT_FAILURE);
    }
} // namespace shl
