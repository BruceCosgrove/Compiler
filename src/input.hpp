#pragma once

#include <filesystem>

namespace shl
{
    struct input
    {
        std::filesystem::path in_path;
        std::filesystem::path out_path;
        std::uint8_t verbose_level = 0;
        std::string_view entry_point = "main";
    };

    const input& handle_input(int argc, char* argv[]);

    const input& get_input() noexcept;
} // namespace shl

#define IF_VERBOSE(level) if (::shl::get_input().verbose_level >= (level))
