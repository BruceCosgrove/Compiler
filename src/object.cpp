#include "object.hpp"
#include <cstdint>

namespace shl
{
    bool object::is_static() const noexcept
    {
        return stack_offset == 0;
    }

    std::string object::get_address() const
    {
        std::string address;
        if (!is_static())
        {
            address = "rbp";
            if (std::ptrdiff_t rsp_offset = std::abs(stack_offset * static_cast<std::ptrdiff_t>(sizeof(std::uint64_t))))
            {
                address += ' ';
                address += (stack_offset > 0 ? '+' : '-');
                address += ' ';
                address += std::to_string(rsp_offset);
            }
        }
        else
        {
            address.reserve(name.size() + 1);
            address = name;
            address += '_';
        }
        return address;
    }
} // namespace shl
