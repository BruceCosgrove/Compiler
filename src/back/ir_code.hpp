#pragma once

#include "common/arena_allocator.hpp"
#include "common/util.hpp"
#include "back/ir_line.hpp"
#include <string>
#include <vector>

namespace shl
{
    class ir_code
    {
    public:
        template <typename T, typename... Args> requires(is_any_of_v<T*, IR_TYPES>)
        T* allocate_line(Args&&... args)
        { return _allocator.allocate<T>(std::forward<Args>(args)...); }

        std::size_t append_line(ir_line&& line);
        void insert_line(std::size_t index, ir_line&& line);
        void remove_line(std::size_t index);

        std::size_t add_object(std::string_view object);
        std::string_view get_object(std::size_t id);

    private:
        // Each IR code line, in sequential order.
        std::vector<ir_line> _lines;

        // Contiguous storage for variable names, constant values, labels, temp vars/labels, etc.
        std::string _storage;

        // Two indices into _storage instead of a std::string_view (whose pointer would be invalidated).
        struct object
        {
            std::size_t begin;
            std::size_t end;
        };

        // List of what objects are in the above storage.
        // This is what the ir_line's index into.
        std::vector<object> _objects;

        // Allocator for ir_line's.
        arena_allocator _allocator{1024 * 1024}; // 1 MiB blocks.
    };
} // namespace shl
