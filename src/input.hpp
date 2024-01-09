#pragma once

#include <filesystem>

namespace shl
{
    struct input
    {
        enum class verbose_level : std::uint8_t
        {
            none,
            comments,
            indentation,

            _count
        };

        std::filesystem::path in_path;
        std::filesystem::path out_path;
        verbose_level verbose_level = verbose_level::none;
        std::string_view entry_point = "main";
    };

    constexpr bool operator>=(decltype(input::verbose_level) lhs, decltype(input::verbose_level) rhs) noexcept
    {
        return static_cast<std::uint8_t>(lhs) >= static_cast<std::uint8_t>(rhs);
    }

    const input& handle_input(int argc, char* argv[]);

    const input& get_input() noexcept;
} // namespace shl

#define IS_VERBOSE(level) (::shl::get_input().verbose_level >= (level))
#define IF_VERBOSE(level) if (IS_VERBOSE(level))
