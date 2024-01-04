#pragma once

#include <optional>
#include <type_traits>

namespace shl
{
    template <typename C>
    concept container =
    requires(const C& container, const typename C::size_type index)
    {
        { container.begin() } -> std::same_as<typename C::const_iterator>;
        { container.end()   } -> std::same_as<typename C::const_iterator>;
    };

    template <container C>
    class consumer
    {
    public:
        using container_type = C;
        using value_type = typename C::value_type;
        using size_type = typename C::size_type;
        using difference_type = typename C::difference_type;
        using const_iterator = typename C::const_iterator;

    public:
        [[nodiscard]] inline explicit consumer(const container_type& container) noexcept(std::is_nothrow_copy_constructible_v<container_type>)
            : _container(container), _it(_container.begin()) {}
        [[nodiscard]] inline explicit consumer(container_type&& container) noexcept(std::is_nothrow_move_constructible_v<container_type>)
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
        [[nodiscard]] std::optional<value_type> peek(difference_type offset = 0) const noexcept
        {
            const_iterator it = std::next(_it, offset);
            if (_container.begin() <= it && it < _container.end())
                return *it;
            return std::nullopt;
        }

        // Return the value at the current index and increments the index.
        // Handles out of bounds checks by terminating, so only call this
        // if you know you have a value, i.e. use peek() to check.
        value_type consume()
        {
            return *_it++;
        }

        // Returns the underlying container.
        [[nodiscard]] const container_type& container() const noexcept
        {
            return _container;
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
        const_iterator _it;
    };
} // namespace shl
