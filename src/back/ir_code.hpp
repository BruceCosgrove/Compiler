#pragma once

#include "common/arena_allocator.hpp"
#include "back/ir_line.hpp"
#include <string>
#include <vector>

namespace shl
{
    class ir_code
    {
    public:

    private:
        std::string _code;
        std::vector<ir_line> _lines;
        arena_allocator _allocator;
    };
} // namespace shl
