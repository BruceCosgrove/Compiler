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
        // If the container has a value at iterator() + offset, returns said value.
        // Otherwise, returns nothing.
        [[nodiscard]] std::optional<value_type> peek(const difference_type offset = 0) const noexcept
        {
            if constexpr (std::contiguous_iterator<const_iterator>)
            {
                const_iterator it = std::next(_it, offset);
                if (_container.begin() <= it && it < _container.end())
                    return *it;
            }
            else
            {
                const_iterator it = _it;
                if (offset)
                {
                    const difference_type direction = offset > 0 ? 1 : -1;
                    while (it != _container.end())
                        std::advance(it, direction);
                }
                if (it != _container.end())
                    return *it;
            }
            return std::nullopt;
        }

        // Return the value at the current iterator and advances the iterator.
        // Handles out of bounds checks by terminating, so only call this
        // if you know you have a value, i.e. use peek() to check.
        value_type consume()
        {
            return *_it++;
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
