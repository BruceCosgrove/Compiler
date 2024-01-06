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
        [[nodiscard]] inline explicit generator(const node_program* root) : _root(root) {}

        [[nodiscard]] std::string operator()();

    private:
        // Correspondence: Single-case grammar rule.

        void generate_program();
        void generate_scope(const node_scope* node);

        // Correspondence: Multi-case grammar rule.

        void generate_statement(const node_statement* node);
        void generate_expression(const node_expression* node);
        void generate_term(const node_term* node);
        void generate_binary_operator(const node_binary_operator* node);

    private:
        [[nodiscard]] std::stringstream& output();
        [[nodiscard]] std::stringstream& push();
        [[nodiscard]] std::stringstream& pop();
        void begin_scope();
        void end_scope();

        void output_label(std::string_view label);
        std::string create_label();

    private:
        struct variable
        {
            std::string_view name;
            std::ptrdiff_t stack_location;
        };

    private:
        [[nodiscard]] std::vector<variable>::iterator get_variable_iterator(std::string_view name);

        const node_program* const _root;
        std::stringstream _output;
        std::size_t _indent_level = 1;
        std::ptrdiff_t _stack_location = 0;
        std::vector<variable> _variables;
        std::vector<std::size_t> _scopes;
        std::size_t _label_count = 0;
    };
} // namespace shl
