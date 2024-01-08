#include "generator.hpp"
#include "error.hpp"
#include <algorithm>
#include <iostream>  // DEBUG
#include <sstream>

// sections:
//  data: initializes read/write memory.
//  bss:
// _output_data << "section .data\n";
// _output_data << "section .bss\n";

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
        output(_output_data, false) << "section .data\n\n";
        for (auto& static_variable : _static_variables)
            _output_data << static_variable.name << ": dq 0\n";

        // Output code.
        output(_output_text, false) << "section .text\n\n";
        generate_start(_output_text);

        for (auto& function : _functions)
            _output_text << '\n' << function.output.rdbuf();

        std::stringstream output;
        output << std::move(_output_data).rdbuf() << '\n';
        output << std::move(_output_text).rdbuf() << '\n';
        return std::move(output).str();
    }

    void generator::generate_program()
    {
        for (auto node : _root->declarations)
            generate_declaration(_output_text, node);
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
                if (variable.in_function())
                {
                    g.generate_expression(out, node->n_expression); // TODO: generate into the following stack offset instead of rax.
                    g.output(out) << "mov [" << g.stack_frame_offset(variable.stack_offset) << "], rax";
                    g.output_verbose_name(out, node->n_name->value) << '\n';
                }
                // else
                    // variable.value = node
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

                auto variable = g.get_variable(node->n_identifier->value);
                if (!variable) error_exit("Undefined variable.");

                g.generate_expression(out, node->n_expression); // TODO: generate into the following stack offset instead of rax.
                g.output(out) << "mov [" << g.stack_frame_offset(variable->stack_offset) << "], rax";
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

                auto variable = g.get_variable(node->value);
                if (!variable) error_exit("Undefined variable.");

                g.output(out) << "mov rax, QWORD [" << g.stack_frame_offset(variable->stack_offset) << "]";
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
        // Verify correct state.

        auto entry_point = get_function_from_name(get_input().entry_point);
        if (!entry_point) error_exit("Entry point not defined.");

        if (entry_point->return_values.size() > 1)
            error_exit("Ill-formed entry point. Incorrect return value count. Must be 0 or 1.");
        if (entry_point->parameters.size() != 2 && !entry_point->parameters.empty())
            error_exit("Ill-formed entry point. Incorrect parameter count. Must be 0 or 2.");

        // Generate start.

        output(out, false) << "global _start\n_start:\n";
        push(out) << '0'; output_verbose_name(out, "status") << '\n';
        output(out) << "mov rbp, rsp\n";

        if (!entry_point->parameters.empty())
        {
            push(out) << "rdi"; output_verbose_name(out, "argc") << '\n';
            push(out) << "rsi"; output_verbose_name(out, "argv") << '\n';
        }

        output(out) << "call " << entry_point->signature << '\n';

        output(out) << "mov rax, 60\n";
        output(out) << "mov rdi, [rbp]\n";
        output(out) << "syscall\n";
    }

    void generator::call_function(std::stringstream& out, std::string_view signature)
    {
        auto function = get_function_from_signature(signature);
        if (!function) error_exit("Unknown function signature.");


        output(out) << "call " << function->signature;
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
        if (get_variable(name))
            error_exit("Redefined variable.");
        auto& variables = get_variables();
        if (has_current_function())
        {
            auto& variable = variables.emplace_back(name, -(1 + variables.size()));
            ++_stack_pointer;
            output(out) << "sub rsp, " << elem_size << '\n';
            return variable;
        }
        else
            return _static_variables.emplace_back(name, 0);
    }

    void generator::deallocate_variable(std::stringstream& out)
    {
        assert(has_current_function());
        auto& function = get_current_function();
        function.variables.pop_back();
        --_stack_pointer;
        output(out) << "add rsp, " << elem_size << '\n';
    }

    auto generator::create_function(const node_named_function* node) -> function&
    {
        std::string signature = get_function_signature(node);

        if (get_function_from_signature(signature))
            error_exit("Redefined function.");

        std::size_t previous_function_index = _current_function_index;
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

        _current_function_index = previous_function_index;
        return function;
    }

    auto generator::get_variable(std::string_view name) -> variable*
    {
        auto check = [=](const variable& variable) { return variable.name == name; };
        variable* variable = nullptr;
        if (has_current_function())
        {
            auto& function = get_current_function();
                 if (auto it = std::ranges::find_if(function.parameters, check); it != function.parameters.end()) variable = it.base();
            else if (auto it = std::ranges::find_if(function.return_values, check); it != function.return_values.end()) variable = it.base();
            else if (auto it = std::ranges::find_if(function.variables, check); it != function.variables.end()) variable = it.base();
        }
        if (!variable)
            if (auto it = std::ranges::find_if(_static_variables, check); it != _static_variables.end()) variable = it.base();
        return variable;
    }

    auto generator::get_function_from_name(std::string_view name) -> function*
    {
        auto check = [=](const function& function) { return function.name == name; };
        if (auto it = std::ranges::find_if(_functions, check); it != _functions.end()) return it.base();
        return nullptr;
    }

    auto generator::get_function_from_signature(std::string_view signature) -> function*
    {
        auto check = [=](const function& function) { return function.signature == signature; };
        if (auto it = std::ranges::find_if(_functions, check); it != _functions.end()) return it.base();
        return nullptr;
    }

    auto generator::get_variables() -> std::vector<variable>&
    {
        return has_current_function() ? get_current_function().variables : _static_variables;
    }

    auto generator::get_current_function() -> function&
    {
        return _functions[_current_function_index];
    }

    bool generator::has_current_function() const noexcept
    {
        return 0 <= _current_function_index;
    }
} // namespace shl
