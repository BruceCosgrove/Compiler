#pragma once

#include "token_nodes.hpp"
#include <sstream>
#include <string>
#include <vector>

namespace shl
{
    class generator
    {
    public:
        [[nodiscard]] inline explicit generator(const node_program* const root) : _root(root) {}

        [[nodiscard]] std::string operator()();

    private:
        void generate_program();
        void generate_statement(const node_statement* const node);
        void generate_expression(const node_expression* const node);
        void generate_term(const node_term* const node);
        void generate_binary_operator(const node_binary_operator* const node);

    private:
        [[nodiscard]] std::stringstream& output();
        [[nodiscard]] std::stringstream& push();
        [[nodiscard]] std::stringstream& pop();
        void begin_scope();
        void end_scope();

    private:
        struct variable
        {
            std::string_view name;
            std::ptrdiff_t stack_location;
        };

    private:
        [[nodiscard]] std::vector<variable>::iterator get_variable_iterator(const std::string_view name);

        const node_program* const _root;
        std::stringstream _output;
        std::size_t _indent_level = 1;
        std::ptrdiff_t _stack_location = 0;
        std::vector<variable> _variables;
        std::vector<std::size_t> _scopes;
    };
} // namespace shl
