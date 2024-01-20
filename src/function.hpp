#pragma once

#include "object.hpp"
#include <vector>
#include <sstream>

namespace shl
{
    struct function
    {
        std::string_view name;
        std::string signature;
        std::vector<object> return_values;
        std::vector<object> parameters;
        std::vector<object> objects;
        std::vector<object> static_objects;
        std::vector<function> nested_functions;
        std::stringstream output;
    };
} // namespace shl
