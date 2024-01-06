#pragma once

#include <filesystem>

namespace shl
{
    struct input
    {
        std::filesystem::path in_path;
        std::filesystem::path out_path;
        bool verbose = false;
    };

    const input& handle_input(int argc, char* argv[]);

    const input& get_input() noexcept;
} // namespace shl

#define IF_VERBOSE if (::shl::get_input().verbose)
