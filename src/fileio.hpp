#pragma once

#include <filesystem>
#include <string>

namespace shl::fileio
{
    [[nodiscard]] bool read(const std::filesystem::path& file_path, std::string& file_contents) noexcept;
    [[nodiscard]] bool write(const std::filesystem::path& file_path, const std::string& file_contents) noexcept;
} // namespace shl::fileio
