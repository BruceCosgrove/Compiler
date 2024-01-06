#pragma once

#include <optional>
#include <type_traits>

namespace shl
{
    template <typename C>
        requires(requires(const C& container)
        {
            { container.begin() } -> std::same_as<typename C::const_iterator>;
            { container.end()   } -> std::same_as<typename C::const_iterator>;
        })
    class consumer
    {
    public:
        using container_type = C;
        using value_type = typename C::value_type;
        using size_type = typename C::size_type;
        using difference_type = typename C::difference_type;
        using const_iterator = typename C::const_iterator;

    public:
        [[nodiscard]] inline explicit consumer(const container_type& container)
            noexcept(std::is_nothrow_copy_constructible_v<container_type>)
            : _container(container), _it(_container.begin()) {}

        [[nodiscard]] inline explicit consumer(container_type&& container)
            noexcept(std::is_nothrow_move_constructible_v<container_type>)
            : _container(std::move(container)), _it(_container.begin()) {}

        consumer(const consumer&) = delete;
        consumer(consumer&&) = delete;
        consumer& operator=(const consumer&) = delete;
        consumer& operator=(consumer&&) = delete;

        [[nodiscard]] consumer() noexcept(std::is_nothrow_default_constructible_v<container_type>) = default;
        ~consumer() noexcept = default;

    protected:
        // If the container has a value at std::next(iterator(), offset), returns said value.
        // Otherwise, returns nothing.
        [[nodiscard]] std::optional<value_type> peek(size_type offset = 0) const noexcept
        {
            size_type max_offset = std::distance(_it, _container.end());
            if (offset < max_offset)
                return *std::next(_it, offset);
            return std::nullopt;
        }

        // Returns the current iterator and advances _it.
        // Call peek() first to see if the advance is valid.
        const_iterator consume(size_type advance = 1)
        {
            const_iterator it = _it;
            std::advance(_it, advance);
            return it;
        }

        // Returns the current iterator.
        [[nodiscard]] const_iterator iterator() const noexcept
        {
            return _it;
        }

        // Resets the iterator so its ready to consume again if need be.
        void reset() noexcept
        {
            _it = _container.begin();
        }

    private:
        const container_type _container;
        const_iterator _it; // Since the container is const, _it won't be invalidated.
    };
} // namespace shl
