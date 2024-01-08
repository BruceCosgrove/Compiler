#include "generator.hpp"
#include "error.hpp"
#include <algorithm>
#include <iostream>  // DEBUG
#include <sstream>

// sections:
//  data: initializes read/write memory.
//  bss:
// _output << "section .data\n";
// _output << "section .bss\n";

// NOTE: How to write to stdout:
// Put the string in the data section (must be before the text section).
// section .data
// string db "Hello world.", 0xA, 0
// ...
// section .text
// ...
// mov rsi, string ; address of string
// mov rdx, 13     ; length of string
// mov rax, 1      ; command = write
// mov rdi, 1      ; select stdout
// syscall         ; call the kernel

namespace shl
{
    std::string generator::operator()()
    {
        generate_program();

        // Output initialized data.
        // output(_output, false) << "section .data\n\n";

        // Output code.
        output(_output, false) << "section .text\n\n";
        generate_start(_output);

        for (auto& function : _functions)
            _output << '\n' << function.output.rdbuf();

        _indent_level = 1;
        _stack_pointer = 0;
        _functions.clear();
        _functions.shrink_to_fit();
        _scopes.clear();
        _scopes.shrink_to_fit();
        _label_count = 0;
        return std::move(_output).str();
    }

    void generator::generate_program()
    {
        for (auto node : _root->declarations)
            generate_declaration(_output, node);
    }

    void generator::generate_declaration(std::stringstream& out, const node_declaration* node)
    {
        struct visitor
        {
            void operator()(const node_definition* node) const
            {
                IF_VERBOSE(2) g.output(out, false) << "definition\n";
                g.generate_definition(out, node);
            }

            void operator()(const node_declare_variable* node) const
            {
                IF_VERBOSE(2) g.output(out, false) << "declare variable\n";
                assert(false && "Not implemented");
                // std::string_view name = node->n_name->value;
                // if (g.get_variable_iterator(name) != g._variables.end())
                //     error_exit("Redeclared identifier.");
                // g._variables.emplace_back(name, g._stack_pointer);
                // g.generate_expression(node->n_expression);
            }

            generator& g;
            std::stringstream& out;
        };
        visit<visitor>(out, "declaration", node->n_value);
    }

    void generator::generate_definition(std::stringstream& out, const node_definition* node)
    {
        struct visitor
        {
            void operator()(const node_define_variable* node) const
            {
                IF_VERBOSE(2) g.output(out, false) << "define variable\n";
                std::string_view name = node->n_name->value;
                auto& variable = g.allocate_variable(out, name);
                g.generate_expression(out, node->n_expression); // TODO: generate into the following stack offset instead of rax.
                g.output(out) << "mov [" << g.stack_frame_offset(variable.stack_offset) << "], rax";
                g.output_verbose_name(out, node->n_name->value) << '\n';
            }

            void operator()(const node_named_function* node) const
            {
                IF_VERBOSE(2) g.output(out, false) << "named function\n";
                g.create_function(node);
            }

            generator& g;
            std::stringstream& out;
        };
        visit<visitor>(out, "definition", node->n_value);
    }

    void generator::generate_statement(std::stringstream& out, const node_statement* node)
    {
        struct visitor
        {
            void operator()(const node_scope* node) const
            {
                IF_VERBOSE(2) g.output(out, false) << "scope\n";
                g.generate_scope(out, node);
            }

            void operator()(const node_return* node) const
            {
                IF_VERBOSE(2) g.output(out, false) << "return\n";
                g.output(out) << "ret\n";
            }

            void operator()(const node_if* node) const
            {
                IF_VERBOSE(2) g.output(out, false) << "if\n";
                g.generate_if(out, node);
            }

            void operator()(const node_reassign* node) const
            {
                IF_VERBOSE(2) g.output(out, false) << "reassign\n";

                auto it = g.get_variable(g.get_current_function(), node->n_identifier->value);
                if (it == g.get_current_function().variables.end())
                    error_exit("Undefined variable.");

                g.generate_expression(out, node->n_expression); // TODO: generate into the following stack offset instead of rax.
                g.output(out) << "mov [" << g.stack_frame_offset(it->stack_offset) << "], rax";
                g.output_verbose_name(out, node->n_identifier->value) << '\n';
            }

            generator& g;
            std::stringstream& out;
        };
        visit<visitor>(out, "statement", node->n_value);
    }

    void generator::generate_scoped_statement(std::stringstream& out, const node_scoped_statement* node)
    {
        struct visitor
        {
            void operator()(const node_statement* node) const
            {
                IF_VERBOSE(2) g.output(out, false) << "statement\n";
                g.generate_statement(out, node);
            }

            void operator()(const node_declaration* node) const
            {
                IF_VERBOSE(2) g.output(out, false) << "declaration\n";
                g.generate_declaration(out, node);
            }

            void operator()(const node_if* node) const
            {
                IF_VERBOSE(2) g.output(out, false) << "if\n";
                g.generate_if(out, node);
            }

            generator& g;
            std::stringstream& out;
        };
        visit<visitor>(out, "scoped statement", node->n_value);
    }

    void generator::generate_expression(std::stringstream& out, const node_expression* node)
    {
        struct visitor
        {
            void operator()(const node_term* node) const
            {
                IF_VERBOSE(2) g.output(out, false) << "term\n";
                g.generate_term(out, node);
            }

            void operator()(const node_binary_expression* node) const
            {
                IF_VERBOSE(2) g.output(out, false) << "binary_expression\n";

                bool expand_lhs = std::holds_alternative<node_binary_expression*>(node->n_expression_lhs->n_value) ||
                    std::holds_alternative<node_parenthesised_expression*>(std::get<node_term*>(node->n_expression_lhs->n_value)->n_value);
                bool expand_rhs = std::holds_alternative<node_binary_expression*>(node->n_expression_rhs->n_value) ||
                    std::holds_alternative<node_parenthesised_expression*>(std::get<node_term*>(node->n_expression_rhs->n_value)->n_value);

                if (!expand_lhs && !expand_rhs) // both are leaves
                {
                    g.generate_expression(out, node->n_expression_rhs);
                    g.output(out) << "mov rbx, rax\n";
                    g.generate_expression(out, node->n_expression_lhs);
                    g.generate_binary_operator(out, node->n_binary_operator);
                }
                else if (!expand_lhs) // lhs is a leaf, but rhs is not
                {
                    g.generate_expression(out, node->n_expression_rhs); // compute rhs first
                    g.push(out) << "rax\n";
                    g.generate_expression(out, node->n_expression_lhs);
                    g.pop(out) << "rbx\n";
                    g.generate_binary_operator(out, node->n_binary_operator);
                }
                else
                {
                    g.generate_expression(out, node->n_expression_lhs);
                    g.push(out) << "rax\n";
                    g.generate_expression(out, node->n_expression_rhs);
                    g.output(out) << "mov rbx, rax\n";
                    g.pop(out) << "rax\n";
                    g.generate_binary_operator(out, node->n_binary_operator);
                }
            }

            generator& g;
            std::stringstream& out;
        };
        visit<visitor>(out, "expression", node->n_value);
    }

    void generator::generate_term(std::stringstream& out, const node_term* node)
    {
        struct visitor
        {
            void operator()(const node_integer_literal* node) const
            {
                IF_VERBOSE(2) g.output(out, false) << "integer_literal\n";
                g.output(out) << "mov rax, " << node->value << '\n';
            }

            void operator()(const node_identifier* node) const
            {
                IF_VERBOSE(2) g.output(out, false) << "identifier\n";

                auto it = g.get_variable(g.get_current_function(), node->value);
                if (it == g.get_current_function().variables.end())
                    error_exit("Undefined variable.");

                g.output(out) << "mov rax, QWORD [" << g.stack_frame_offset(it->stack_offset) << "]";
                g.output_verbose_name(out, node->value) << '\n';
            }

            void operator()(const node_parenthesised_expression* node) const
            {
                IF_VERBOSE(2) g.output(out, false) << "parenthesised expression\n";
                g.generate_expression(out, node->n_expression);
            }

            generator& g;
            std::stringstream& out;
        };
        visit<visitor>(out, "term", node->n_value);
    }

    void generator::generate_binary_operator(std::stringstream& out, const node_binary_operator *node)
    {
        struct visitor
        {
            void operator()(const node_forward_slash& node) const
            {
                IF_VERBOSE(2) g.output(out, false) << "/\n";
                g.output(out) << "div rbx\n";
            }

            void operator()(const node_percent& node) const
            {
                IF_VERBOSE(2) g.output(out, false) << "%\n";
                g.output(out) << "div rbx\n";
                g.output(out) << "mov rax, rdx\n"; // div puts reg1 % reg2 in rdx
            }

            void operator()(const node_asterisk& node) const
            {
                IF_VERBOSE(2) g.output(out, false) << "*\n";
                g.output(out) << "mul rbx\n";
            }

            void operator()(const node_plus& node) const
            {
                IF_VERBOSE(2) g.output(out, false) << "+\n";
                g.output(out) << "add rax, rbx\n";
            }

            void operator()(const node_minus& node) const
            {
                IF_VERBOSE(2) g.output(out, false) << "-\n";
                g.output(out) << "sub rax, rbx\n";
            }

            generator& g;
            std::stringstream& out;
        };
        visit<visitor>(out, "binary operator", node->n_value);
    }

    void generator::generate_if(std::stringstream& out, const node_if* node)
    {
        generate_expression(out, node->n_expression);
        std::string label_end_if = create_label();
        output(out) << "test rax, rax";
        output(out) << "jz " << label_end_if << '\n';
        generate_statement(out, node->n_statement);
        output_label(out, label_end_if);
    }

    void generator::generate_scope(std::stringstream& out, const node_scope* node)
    {
        begin_scope();
        for (auto n_scoped_statement : node->scoped_statements)
            generate_scoped_statement(out, n_scoped_statement);
        end_scope(out);
    }

    std::stringstream& generator::output(std::stringstream& out, bool indent)
    {
        if (indent)
        {
            IF_VERBOSE(2)
            {
                for (std::size_t i = 0; i < _indent_level; ++i)
                    out << "    ";
            }
            else
                out << "    ";
        }
        return out;
    }

    std::stringstream& generator::push(std::stringstream& out)
    {
        ++_stack_pointer;
        output(out) << "push ";
        return out;
    }

    std::stringstream& generator::pop(std::stringstream& out)
    {
        --_stack_pointer;
        output(out) << "pop ";
        return out;
    }

    std::stringstream& generator::output_label(std::stringstream& out, std::string_view label)
    {
        IF_VERBOSE(2)
            for (std::size_t i = 0; i < _indent_level - 1; ++i)
                out << "    ";
        out << label << ":\n";
        return out;
    }

    std::stringstream& generator::output_verbose_name(std::stringstream& out, std::string_view name)
    {
        IF_VERBOSE(1) output(out, false) << " ; " << name;
        return out;
    }

    std::string generator::stack_frame_offset(std::ptrdiff_t offset)
    {
        std::string string = "rbp";
        if (std::ptrdiff_t rsp_offset = std::abs(offset * elem_size))
        {
            string += ' ';
            string += (offset > 0 ? '+' : '-');
            string += ' ';
            string += std::to_string(rsp_offset);
        }
        return string;
    }

    void generator::begin_scope()
    {
        _scopes.push_back(get_current_function().variables.size());
    }

    void generator::end_scope(std::stringstream& out)
    {
        auto& variables = get_current_function().variables;
        std::size_t pop_count = variables.size() - _scopes.back();
        if (pop_count > 0)
        {
            output(out) << "add rsp, " << (pop_count * elem_size) << '\n';
            _stack_pointer -= pop_count;
            variables.erase(variables.end() - pop_count, variables.end());
        }
        _scopes.pop_back();
    }

    void generator::generate_start(std::stringstream& out)
    {
        auto it = get_function_from_name(get_input().entry_point);
        if (it == _functions.end())
            error_exit("Entry point not defined.");
        auto& entry_point = *it;

        if (entry_point.return_values.size() > 1)
            error_exit("Ill-formed entry point. Incorrect return value count. Must be 0 or 1.");
        if (entry_point.parameters.size() != 2 && !entry_point.parameters.empty())
            error_exit("Ill-formed entry point. Incorrect parameter count. Must be 0 or 2.");

        output(out, false) << "global _start\n_start:\n";
        push(out) << '0'; output_verbose_name(out, "status") << '\n';
        output(out) << "mov rbp, rsp\n";

        if (entry_point.parameters.size() == 2)
        {
            push(out) << "rdi"; output_verbose_name(out, "argc") << '\n';
            push(out) << "rsi"; output_verbose_name(out, "argv") << '\n';
        }

        output(out) << "call " << entry_point.signature << '\n';

        output(out) << "mov rax, 60\n";
        output(out) << "mov rdi, [rbp]\n";
        output(out) << "syscall\n";
    }

    void generator::call_function(std::stringstream& out, std::string_view signature) {
        auto it = get_function_from_signature(signature);
        if (it == _functions.end())
        error_exit("Unknown function signature.");
        auto &function = *it;

        // allocate/push all return values
        // allocate/push all parameters
        // call name
        // deallocate/pop all parameters in reverse order
        // deallocate/pop ignored return values in reverse order
        // TODO: optimization: if some return values are ignored, deallocate them
        //  in reverse order of which they are defined, until either one is
        //  reached that is not ignored, or there are none left. The latter case
        //  is equivalent to not capturing the return values at all. This
        //  deallocation stack offset can be added to the sum of the parameters'
        //  sizes to deallocate all in one go. Remember to also call all their
        //  destructors.

        for (auto &return_value : function.return_values)
            allocate_variable(out, return_value.name);
        for (auto &parameter : function.parameters)
            allocate_variable(out, parameter.name);

        output(out) << "call " << function.signature;

        for (std::size_t i = 0; i < function.return_values.size(); ++i)
            deallocate_variable(out);
        for (std::size_t i = 0; i < function.parameters.size(); ++i)
            deallocate_variable(out);
    }

    std::string generator::create_label()
    {
        return "label" + std::to_string(_label_count++);
    }

    std::string generator::get_function_signature(const node_named_function* node)
    {
        std::stringstream ssignaure;
        ssignaure << node->n_name->value;

        ssignaure << '_';
        for (auto n_return_value : node->n_function->return_values)
            ssignaure << '_' << n_return_value->n_name->value; // TODO: When types are implemented, change "n_name" to "n_type".

        ssignaure << '_';
        for (auto n_parameter : node->n_function->parameters)
            ssignaure << '_' << n_parameter->n_name->value; // TODO: When types are implemented, change "n_name" to "n_type".

        std::string signature = ssignaure.str();
        // Change asterisks to periods to appease assembler.
        for (char& c : signature)
            if (c == '*')
                c = '.';
        return signature;
    }

    auto generator::allocate_variable(std::stringstream& out, std::string_view name) -> variable&
    {
        auto& function = get_current_function();
        if (get_variable(function, name) != function.variables.end())
            error_exit("Redefined variable.");
        auto& variable = function.variables.emplace_back(name, -(1 + function.variables.size()));
        ++_stack_pointer;
        output(out) << "sub rsp, " << elem_size << '\n';
        return variable;
    }

    void generator::deallocate_variable(std::stringstream& out)
    {
        auto& function = get_current_function();
        function.variables.pop_back();
        --_stack_pointer;
        output(out) << "add rsp, " << elem_size << '\n';
    }

    auto generator::create_function(const node_named_function* node) -> function&
    {
        std::string signature = get_function_signature(node);

        if (get_function_from_signature(signature) != _functions.end())
            error_exit("Redefined function.");

        _current_function_index = _functions.size();
        auto& function = _functions.emplace_back(node->n_name->value, std::move(signature));

        std::ptrdiff_t return_value_count = node->n_function->return_values.size();
        std::ptrdiff_t parameter_count = node->n_function->parameters.size();
        std::ptrdiff_t stack_offset = return_value_count + parameter_count + 1; // + 1 only if push rbp

        // Convert the return values.
        function.return_values.reserve(return_value_count);
        for (auto n_return_value : node->n_function->return_values)
            function.return_values.emplace_back(n_return_value->n_name->value, stack_offset--);
        // Convert the parameters.
        function.parameters.reserve(parameter_count);
        for (auto n_parameter : node->n_function->parameters)
            function.parameters.emplace_back(n_parameter->n_name->value, stack_offset--);

        output_label(function.output, function.signature);
        push(function.output) << "rbp\n";
        output(function.output) << "mov rbp, rsp\n";
        generate_statement(function.output, node->n_function->n_statement);
        pop(function.output) << "rbp\n";
        output(function.output) << "ret\n";
        return function;
    }

    auto generator::get_variable(function& function, std::string_view name) -> std::vector<variable>::iterator
    {
        auto check = [=](const variable& variable) { return variable.name == name; };
        if (auto it = std::ranges::find_if(function.parameters, check); it != function.parameters.end()) return it;
        if (auto it = std::ranges::find_if(function.return_values, check); it != function.return_values.end()) return it;
        return std::ranges::find_if(function.variables, check);
    }

    auto generator::get_function_from_name(std::string_view name) -> std::vector<function>::iterator
    {
        auto check = [=](const function& function) { return function.name == name; };
        return std::ranges::find_if(_functions, check);
    }

    auto generator::get_function_from_signature(std::string_view signature) -> std::vector<function>::iterator
    {
        auto check = [=](const function& function) { return function.signature == signature; };
        return std::ranges::find_if(_functions, check);
    }

    auto generator::get_current_function() -> function&
    {
        return _functions[_current_function_index];
    }
} // namespace shl
