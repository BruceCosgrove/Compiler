#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace shl
{
    class arena_allocator
    {
    public:
        arena_allocator(const arena_allocator&) = delete;
        arena_allocator(arena_allocator&&) = delete;
        arena_allocator& operator=(const arena_allocator&) = delete;
        arena_allocator& operator=(arena_allocator&&) = delete;

        [[nodiscard]] inline explicit arena_allocator(std::size_t block_size) noexcept
            : _block_size(block_size)
        {
            allocate_new_buffer();
        }

        ~arena_allocator() noexcept
        {
            for (std::uint8_t* block : _blocks)
                delete[] block;
        }

        template <typename T, typename... Args> requires(std::is_constructible_v<T, Args...>)
        [[nodiscard]] T* allocate(Args&&... construct_args)
        {
            void* object = _blocks.back() + (_block_size - _block_size_remaining);
            object = std::align(alignof(T), sizeof(T), object, _block_size_remaining);
            if (!object || _block_size_remaining < sizeof(T))
                allocate_new_buffer();
            _block_size_remaining -= sizeof(T);
            return new(object) T(std::forward<Args>(construct_args)...);
        }

    private:
        inline void allocate_new_buffer()
        {
            _block_size_remaining = _block_size;
            _blocks.push_back(new std::uint8_t[_block_size]);
        }

        std::size_t _block_size;
        std::size_t _block_size_remaining;
        std::vector<std::uint8_t*> _blocks;
    };
} // namespace shl
