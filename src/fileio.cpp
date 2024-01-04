#include "fileio.hpp"
#include <fstream>
#include <sstream>

namespace shl::fileio
{
    bool read(const std::filesystem::path& file_path, std::string& file_contents) noexcept
    {
        try
        {
            std::ifstream file_stream(file_path);
            if (file_stream)
            {
                std::stringstream content_stream;
                content_stream << file_stream.rdbuf();
                file_contents = content_stream.str();
                return file_stream.good();
            }
        }
        catch(...) {}
        return false;
    }

    bool write(const std::filesystem::path& file_path, const std::string& file_contents, bool append) noexcept
    {
        try
        {
            std::ofstream file_stream(file_path, append ? std::ios::ate : std::ios::trunc);
            if (file_stream)
            {
                file_stream.write(file_contents.c_str(), file_contents.size());
                return file_stream.good();
            }
        }
        catch(...) {}
        return false;
    }
} // namespace shl::fileio
