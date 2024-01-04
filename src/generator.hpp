#pragma once

#include "token_nodes.hpp"
#include <sstream>
#include <string>
#include <unordered_map>

namespace shl
{
    class generator
    {
    public:
        [[nodiscard]] inline explicit generator(const node_program& root) : _root(root) {}
        [[nodiscard]] inline explicit generator(node_program&& root) : _root(std::move(root)) {}

        [[nodiscard]] std::string generate();

    private: // Each of these correspond to a non-trivial (not entirely regex) grammar rule.
        void generate_program();
        void generate_statement(const node_statement& node);
        void generate_expression(const node_expression& node);

    private:
        std::stringstream& output();
        std::stringstream& push();
        std::stringstream& pop();

    private:
        struct variable
        {
            std::size_t stack_location;
        };

        const node_program _root;
        std::stringstream _output;
        std::size_t _indent_level = 1;
        std::size_t _stack_location = 0;
        std::unordered_map<std::string_view, variable> _variables;
    };
} // namespace shl
