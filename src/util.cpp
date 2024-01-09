#include "util.hpp"

namespace shl
{
    std::string insert_after_each(std::string_view string, std::string_view after)
    {
        std::string result;
        result.reserve(string.size() * (1 * after.size()));
        for (char c : string)
            (result += c) += after;
        return result;
    }
} // namespace shl
