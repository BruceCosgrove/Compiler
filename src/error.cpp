#include "error.hpp"
#include <iostream>

namespace shl
{
    void error_exit(const std::string_view error_message)
    {
        std::cerr << "Error: " << error_message << std::endl;
        exit(EXIT_FAILURE);
    }
} // namespace shl
