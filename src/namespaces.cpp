#include "namespaces.hpp"

namespace shl
{
    void namespaces::push_namespace(std::string_view new_namespace)
    {
        namespace_indices.push_back(current_namespace.size());
        current_namespace += new_namespace;
    }

    void namespaces::pop_namespace()
    {
        current_namespace.erase(current_namespace.begin() + namespace_indices.back(), current_namespace.end());
        namespace_indices.pop_back();
    }
} // namespace shl
