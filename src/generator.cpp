#include "generator.hpp"
#include "error.hpp"
#include <algorithm>

namespace shl
{
    std::string generator::operator()()
    {
        generate_program();

        std::string assembly = _output.str();
        _output = {};
        return assembly;
    }

    void generator::generate_program()
    {
        _output << "global _start\n_start:\n";

        for (const node_statement* const node : _root->_statements)
            generate_statement(node);

        output() << "mov rax, 60\n";
        output() << "mov rdi, 0\n";
        output() << "syscall\n";
    }

    void generator::generate_statement(const node_statement* const node)
    {
        struct
        {
            void operator()(const node_return* const node) const
            {
                g._output << "return\n";
                g.generate_expression(node->_expression);
                g.output() << "mov rax, 60\n";
                g.pop() << "rdi\n";
                g.output() << "syscall\n";
            }

            void operator()(const node_declare_identifier* const node) const
            {
                g._output << "declare_identifier\n";
                const std::string_view name = *node->_identifier->_token.value;
                if (g.get_variable_iterator(name) != g._variables.end())
                    error_exit("Redeclared identifier.");
                g._variables.emplace_back(name, g._stack_location);
                g.generate_expression(node->_expression);
            }

            void operator()(const node_scope* const node) const
            {
                g._output << "scope\n";
            }

            generator& g;
        } visitor(*this);

        output() << "; statement: ";
        ++_indent_level;
        std::visit(visitor, node->value);
        --_indent_level;
    }

    void generator::generate_expression(const node_expression* const node)
    {
        struct
        {
            void operator()(const node_term* const node) const
            {
                g._output << "term\n";
                g.generate_term(node);
            }

            void operator()(const node_binary_expression* const node) const
            {
                g._output << "binary_expression\n";
                g.generate_expression(node->_expression_left);
                g.generate_expression(node->_expression_right);
                g.pop() << "rbx\n";
                g.pop() << "rax\n";
                g.generate_binary_operator(node->_binary_operator);
                g.push() << "rax\n";
            }

            generator& g;
        } visitor(*this);

        output() << "; expression: ";
        ++_indent_level;
        std::visit(visitor, node->value);
        --_indent_level;
    }

    void generator::generate_term(const node_term* const node)
    {
        struct
        {
            void operator()(const node_integer_literal* const node) const
            {
                g._output << "integer_literal\n";
                g.push() << "QWORD " << *node->_token.value << '\n';
            }

            void operator()(const node_identifier* const node) const
            {
                g._output << "identifier\n";
                const auto it = g.get_variable_iterator(*node->_token.value);
                if (it == g._variables.end())
                    error_exit("Undeclared identifier in current scope.");
                const std::ptrdiff_t stack_location = (g._stack_location - it->stack_location - 1) * sizeof(std::uint64_t);
                g.push() << "QWORD [rsp + " << stack_location << "]\n";
            }

            void operator()(const node_parenthesised_expression* const node) const
            {
                g._output << "parenthesised expression\n";
                g.generate_expression(node->_expression);
            }

            generator& g;
        } visitor(*this);

        output() << "; term: ";
        ++_indent_level;
        std::visit(visitor, node->value);
        --_indent_level;
    }

    void generator::generate_binary_operator(const node_binary_operator *const node)
    {
        struct
        {
            void operator()(const node_forward_slash* const node) const
            {
                g._output << "/\n";
                g.output() << "div rbx\n";
            }

            void operator()(const node_percent* const node) const
            {
                g._output << "%\n";
                g.output() << "div rbx\n";
                g.output() << "mov rax, rdx\n";
            }

            void operator()(const node_asterisk* const node) const
            {
                g._output << "*\n";
                g.output() << "mul rbx\n";
            }

            void operator()(const node_plus* const node) const
            {
                g._output << "+\n";
                g.output() << "add rax, rbx\n";
            }

            void operator()(const node_minus* const node) const
            {
                g._output << "-\n";
                g.output() << "sub rax, rbx\n";
            }

            generator& g;
        } visitor(*this);

        output() << "; binary operator: ";
        ++_indent_level;
        std::visit(visitor, node->value);
        --_indent_level;
    }

    std::stringstream& generator::output()
    {
        for (std::size_t i = 0; i < _indent_level; ++i)
            _output << "    ";
        return _output;
    }

    std::stringstream& generator::push()
    {
        output() << "push ";
        ++_stack_location;
        return _output;
    }

    std::stringstream& generator::pop()
    {
        output() << "pop ";
        --_stack_location;
        return _output;
    }

    auto generator::get_variable_iterator(const std::string_view name) -> std::vector<variable>::iterator
    {
        return std::ranges::find_if(_variables, [=](const variable& variable) { return variable.name == name; });
    }
} // namespace shl
