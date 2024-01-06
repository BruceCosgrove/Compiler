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
            // Destruct all the objects.
            for (object_metadata& object : _objects)
                object.destructor(object.address);
            // Deallocate all the blocks.
            for (std::uint8_t* block : _blocks)
                delete[] block;
        }

        template <typename T, typename... Args> requires(std::is_constructible_v<T, Args...>)
        [[nodiscard]] T* allocate(Args&&... construct_args)
        {
            std::size_t size_remaning_before = _block_size_remaining;

            // Get the object's address and size (accounting for alignment).
            void* address = _blocks.back() + (_block_size - _block_size_remaining);
            address = std::align(alignof(T), sizeof(T), address, _block_size_remaining);
            if (!address || _block_size_remaining < sizeof(T))
                allocate_new_buffer();
            _block_size_remaining -= sizeof(T);

            // Create the object metadata.
            std::size_t size_remaning_after = _block_size_remaining;
            std::size_t object_size = size_remaning_before - size_remaning_after;
            _objects.emplace_back(address, object_size, [](const void* address) noexcept { static_cast<const T*>(address)->~T(); });

            // Create and return the object.
            return new(address) T(std::forward<Args>(construct_args)...);
        }

    private:
        inline void allocate_new_buffer()
        {
            _block_size_remaining = _block_size;
            _blocks.push_back(new std::uint8_t[_block_size]);
        }

        struct object_metadata
        {
            void* address;
            std::size_t size;
            void(*destructor)(const void* address) noexcept;
        };

        std::size_t _block_size;
        std::size_t _block_size_remaining;
        std::vector<std::uint8_t*> _blocks;
        std::vector<object_metadata> _objects;
    };
} // namespace shl
