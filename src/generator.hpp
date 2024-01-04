#pragma once

#include "token_nodes.hpp"
#include <sstream>
#include <string>

namespace shl
{
    class generator
    {
    public:
        [[nodiscard]] inline explicit generator(const node_return& root) : _root(root) {}
        [[nodiscard]] inline explicit generator(node_return&& root) : _root(std::move(root)) {}

        [[nodiscard]] std::string generate();

    private:
        void generate_return();

    private:
        const node_return _root;
        std::stringstream _output;

    private:
        static constexpr std::string_view indent = "    ";
    };
} // namespace shl
