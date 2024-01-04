#pragma once

#include <optional>
#include <type_traits>

namespace shl
{
    template <typename CC>
    concept contiguous_container =
    requires(const CC& container, const typename CC::size_type index)
    {
        { container.size()    } -> std::same_as<typename CC::size_type>;
        { container.at(index) } -> std::same_as<typename CC::const_reference>;
        { container.begin()   } -> std::same_as<typename CC::const_iterator>;
        { container.end()     } -> std::same_as<typename CC::const_iterator>;
    };

    template <contiguous_container CC>
    class consumer
    {
    public:
        using container_type = CC;
        using value_type = typename CC::value_type;
        using size_type = typename CC::size_type;
        using difference_type = typename CC::difference_type;
        using const_iterator = typename CC::const_iterator;

    public:
        [[nodiscard]] inline explicit consumer(const container_type& container) noexcept(std::is_nothrow_copy_constructible_v<container_type>) : _container(container) {}
        [[nodiscard]] inline explicit consumer(container_type&& container) noexcept(std::is_nothrow_move_constructible_v<container_type>) : _container(std::move(container)) {}

        consumer(const consumer&) = delete;
        consumer(consumer&&) = delete;
        consumer& operator=(const consumer&) = delete;
        consumer& operator=(consumer&&) = delete;

        [[nodiscard]] consumer() noexcept(std::is_nothrow_default_constructible_v<container_type>) = default;
        ~consumer() noexcept = default;

    protected:
        // If the container has a value at index "index() + offset", returns said value.
        // Otherwise, returns nothing.
        [[nodiscard]] std::optional<value_type> peek(difference_type offset = 0) const
        {
            size_type index = _index + offset;
            if (index >= _container.size())
                return std::nullopt;
            return _container.at(index);
        }

        // Return the value at the current index and increments the index.
        // Handles out of bounds checks by terminating, so only call this
        // if you know you have a value, i.e. use peek() to check.
        value_type consume()
        {
            return _container.at(_index++);
        }

        // Returns the underlying container.
        [[nodiscard]] const container_type& container() const noexcept
        {
            return _container;
        }

        // Returns the current index.
        [[nodiscard]] size_type index() const noexcept
        {
            return _index;
        }

        // Resets the index so its ready to consume again if need be.
        void reset() noexcept
        {
            _index = 0;
        }

    private:
        const container_type _container;
        size_type _index = 0;
    };
} // namespace shl
