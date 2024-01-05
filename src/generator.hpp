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
        [[nodiscard]] inline explicit generator(const node_program* const root) : _root(root) {}

        [[nodiscard]] std::string operator()();

    private: // Each of these correspond to a non-trivial (not entirely regex) grammar rule.
        void generate_program();
        void generate_statement(const node_statement* const node);
        void generate_expression(const node_expression* const node);
        void generate_term(const node_term* const node);

    private:
        std::stringstream& output();
        std::stringstream& push();
        std::stringstream& pop();

    private:
        struct variable
        {
            std::size_t stack_location;
        };

        const node_program* const _root;
        std::stringstream _output;
        std::size_t _indent_level = 1;
        std::size_t _stack_location = 0;
        std::unordered_map<std::string_view, variable> _variables;
    };
} // namespace shl
