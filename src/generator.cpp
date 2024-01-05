#include "generator.hpp"
#include "error.hpp"

namespace shl
{
    std::string generator::generate()
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
                if (g._variables.contains(*node->_identifier->_token.value))
                    error_exit("Redeclared identifier.");
                auto& variable = g._variables[*node->_identifier->_token.value];
                variable.stack_location = g._stack_location;
                g.generate_expression(node->_expression);
            }

            generator& g;
        } visitor(*this);

        output() << "; statement: ";
        ++_indent_level;
        std::visit(visitor, node->_statement);
        --_indent_level;
    }

    void generator::generate_expression(const node_expression* const node)
    {
        struct
        {
            void operator()(const node_integer_literal* const node) const
            {
                g._output << "integer_literal\n";
                g.output() << "mov rax, " << *node->_token.value << '\n';
                g.push() << "rax\n";
            }

            void operator()(const node_identifier* const node) const
            {
                g._output << "identifier\n";
                const auto it = g._variables.find(*node->_token.value);
                if (it == g._variables.end())
                    error_exit("Undeclared identifier.");
                const auto& variable = it->second;
                const std::int64_t stack_location = (g._stack_location - variable.stack_location - 1) * sizeof(std::int64_t);
                g.push() << "QWORD [rsp + " << stack_location << "]\n";
            }

            void operator()(const node_binary_expression* const node) const
            {
                error_exit("node_binary_expression unimplemented.");
            }

            generator& g;
        } visitor(*this);

        output() << "; expression: ";
        ++_indent_level;
        std::visit(visitor, node->_expression);
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
} // namespace shl
