#include "ir_code.hpp"

namespace shl
{
    std::size_t ir_code::append_line(ir_line&& line)
    {
        std::size_t index = _lines.size();
        _lines.push_back(std::move(line));
        return index;
    }

    void ir_code::insert_line(std::size_t index, ir_line&& line)
    {
        _lines.insert(_lines.begin() + index, std::move(line));
    }

    void ir_code::remove_line(std::size_t index)
    {
        _lines.erase(_lines.begin() + index);
    }

    std::size_t ir_code::add_object(std::string_view object)
    {
        std::size_t id = _objects.size();

        if (object.empty())
        {
            _storage += "__T";
            _storage += std::to_string(id);
        }
        else
            _storage += object;

        _objects.emplace_back(_storage.size() - object.size(), _storage.size());
        return id;
    }

    std::string_view ir_code::get_object(std::size_t id)
    {
        auto& object = _objects.at(id);
        return std::string_view(_storage.begin() + object.begin, _storage.begin() + object.end);
    }
} // namespace shl
