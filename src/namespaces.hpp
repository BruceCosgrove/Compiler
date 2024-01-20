#pragma once

#include <string>
#include <vector>

namespace shl
{
    class namespaces
    {
    public:
        void push_namespace(std::string_view new_namespace);
        void pop_namespace();

        inline const std::string& get_namespace() const noexcept { return current_namespace; }
        inline bool empty() const noexcept { return current_namespace.empty(); }
    private:
        // The entire current namespace.
        std::string current_namespace;
        // Indices into current_namespace dictating where each namespace starts.
        std::vector<std::size_t> namespace_indices;
    };
} // namespace shl
