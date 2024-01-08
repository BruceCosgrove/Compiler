#include "generator.hpp"
#include "error.hpp"
#include "util.hpp"
#include <algorithm>
#include <iostream>  // DEBUG
#include <sstream>
#include <utility>

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

#define VERBOSE_OUT(level, to_output, ...) \
        (IS_VERBOSE(level) ? static_cast<std::stringstream&>(__VA_OPT__(g.)output(false) << to_output) : (*__VA_OPT__(g.)_output_current))

#define VERBOSE_COMMENT(level, to_output, ...) VERBOSE_OUT((level), " ; " << to_output __VA_OPT__(,) __VA_ARGS__)

namespace shl
{
    struct generator_visitor
    {
        generator& g; // context

        void operator()(const node_program* node)
        {
            for (auto node : node->declarations)
                g.visit(this, "program", cast_variant<NODE_TYPES>(node->n_value));
        }

        void operator()(const node_declaration* node)
        {
            VERBOSE_OUT(2, "declaration\n", true);
            g.visit(this, "declaration", cast_variant<NODE_TYPES>(node->n_value));
        }

        void operator()(const node_definition* node)
        {
            VERBOSE_OUT(2, "definition\n", true);
            g.visit(this, "definition", cast_variant<NODE_TYPES>(node->n_value));
        }

        void operator()(const node_declare_object* node)
        {
            VERBOSE_OUT(2, "declare object\n", true);
            assert(false && "declare object unimplemented.");
        }

        void operator()(const node_define_object* node)
        {
            VERBOSE_OUT(2, "define object\n", true);
            std::string_view name = node->n_name->value;
            auto& object = g.allocate_object(name);
            if (object.in_function())
            {
                g.visit(this, "expression", cast_variant<NODE_TYPES>(node->n_expression->n_value));
                g.output() << "mov [" << object.get_address() << "], rax";
                VERBOSE_COMMENT(1, node->n_name->value, true) << '\n';
            }
            else
                assert(false && "define static object unimplemented.");
        }

        void operator()(const node_function* node)
        {
            VERBOSE_OUT(2, "function\n", true);
            assert(false && "function unimplemented.");
        }

        void operator()(const node_named_function* node)
        {
            VERBOSE_OUT(2, "named function\n", true);

            std::string signature = g.get_function_signature(node);
            if (g.get_function_from_signature(signature)) error_exit("Redefined function.");
            auto& function = g._functions.emplace_back(node->n_name->value, std::move(signature));

            // Make the function output to its own stream.
            std::stringstream* output_previous = std::exchange(g._output_current, &function.output);
            std::ptrdiff_t previous_function_index = std::exchange(g._current_function_index, static_cast<std::ptrdiff_t>(g._functions.size() - 1));

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

            g.output_label(function.signature) << '\n';
            g.push() << "rbp\n";
            g.output() << "mov rbp, rsp\n";
            g.visit(this, "statement", cast_variant<NODE_TYPES>(node->n_function->n_statement->n_value));
            g.pop() << "rbp\n";
            g.output() << "ret\n";

            // Restore the old stream.
            std::swap(g._output_current, output_previous);
            std::swap(g._current_function_index, previous_function_index);
        }

        void operator()(const node_parameter* node)
        {
            VERBOSE_OUT(2, "parameter\n", true);
            assert(false && "parameter unimplemented.");
        }

        void operator()(const node_scope* node)
        {
            VERBOSE_OUT(2, "scope\n", true);
            g.begin_scope();
            for (auto n_scoped_statement : node->scoped_statements)
                g.visit(this, "scoped statement", cast_variant<NODE_TYPES>(n_scoped_statement->n_value));
            g.end_scope();
        }

        void operator()(const node_statement* node)
        {
            VERBOSE_OUT(2, "statement\n", true);
            g.visit(this, "statement", cast_variant<NODE_TYPES>(node->n_value));
        }

        void operator()(const node_scoped_statement* node)
        {
            VERBOSE_OUT(2, "scoped statement\n", true);
            g.visit(this, "scoped statement", cast_variant<NODE_TYPES>(node->n_value));
        }

        void operator()(const node_expression* node)
        {
            VERBOSE_OUT(2, "expression\n", true);
            g.visit(this, "expression", cast_variant<NODE_TYPES>(node->n_value));
        }

        void operator()(const node_term* node)
        {
            VERBOSE_OUT(2, "term\n", true);
            g.visit(this, "term", cast_variant<NODE_TYPES>(node->n_value));
        }

        void operator()(const node_return* node)
        {
            VERBOSE_OUT(2, "return\n", true);
            g.pop() << "rbp\n";
            g.output() << "ret\n";
        }

        void operator()(const node_if* node)
        {
            VERBOSE_OUT(2, "if\n", true);
            g.visit(this, "expression", cast_variant<NODE_TYPES>(node->n_expression->n_value));
            std::string label_end_if = g.create_label();
            g.output() << "test rax, rax\n";
            g.output() << "jz " << label_end_if << '\n';
            g.visit(this, "statement", cast_variant<NODE_TYPES>(node->n_statement->n_value));
            g.output_label(label_end_if);
            VERBOSE_COMMENT(1, "endif", true) << '\n';
        }

        void operator()(const node_reassign* node)
        {
            VERBOSE_OUT(2, "reassign\n", true);

            auto object = g.get_object(node->n_identifier->value);
            if (!object) error_exit("Undefined object.");

            if (object->in_function())
            {
                g.visit(this, "expression", cast_variant<NODE_TYPES>(node->n_expression->n_value));
                g.output() << "mov [" << object->get_address() << "], rax";
                VERBOSE_COMMENT(1, node->n_identifier->value, true) << '\n';
            }
            else
                assert(false && "reassign static object unimplemented.");
        }

        void operator()(const node_scoped_if* node)
        {
            VERBOSE_OUT(2, "scoped if\n", true);

            std::string label_end_if = g.create_label();
            std::string label_next_if = node->ifs.size() == 1 ? label_end_if : g.create_label(); // Copy ok, won't allocate.

            std::size_t i = 0;
            do
            {
                auto n_if = node->ifs[i++];
                if (n_if->n_expression) // Only else blocks won't enter here.
                {
                    g.visit(this, "expression", cast_variant<NODE_TYPES>(n_if->n_expression->n_value));
                    g.output() << "test rax, rax\n";
                    g.output() << "jz " << label_next_if << '\n';
                }
                g.visit(this, "statement", cast_variant<NODE_TYPES>(n_if->n_statement->n_value));
                if (i < node->ifs.size())
                {
                    g.output() << "jmp " << label_end_if << '\n';
                    g.output_label(label_next_if);
                    VERBOSE_COMMENT(1, (node->ifs[i]->n_expression ? "elif" : "else"), true) << '\n';
                    label_next_if = i + 1 < node->ifs.size() ? g.create_label() : label_end_if;
                }
            }
            while (i < node->ifs.size());

            g.output_label(label_end_if);
            VERBOSE_COMMENT(1, "endif", true) << '\n';
        }

        void operator()(const node_binary_expression* node)
        {
            VERBOSE_OUT(2, "binary expression\n", true);

            bool expand_lhs = std::holds_alternative<node_binary_expression*>(node->n_expression_lhs->n_value) ||
                std::holds_alternative<node_parenthesised_expression*>(std::get<node_term*>(node->n_expression_lhs->n_value)->n_value);
            bool expand_rhs = std::holds_alternative<node_binary_expression*>(node->n_expression_rhs->n_value) ||
                std::holds_alternative<node_parenthesised_expression*>(std::get<node_term*>(node->n_expression_rhs->n_value)->n_value);

            if (!expand_lhs && !expand_rhs) // both are leaves
            {
                g.visit(this, "expression", cast_variant<NODE_TYPES>(node->n_expression_rhs->n_value));
                g.output() << "mov rbx, rax\n";
                g.visit(this, "expression", cast_variant<NODE_TYPES>(node->n_expression_lhs->n_value));
                g.visit(this, "binary operator", cast_variant<NODE_TYPES>(node->n_binary_operator->n_value));
            }
            else if (!expand_lhs) // lhs is a leaf, but rhs is not
            {
                g.visit(this, "expression", cast_variant<NODE_TYPES>(node->n_expression_rhs->n_value)); // compute rhs first
                g.push() << "rax\n";
                g.visit(this, "expression", cast_variant<NODE_TYPES>(node->n_expression_lhs->n_value));
                g.pop() << "rbx\n";
                g.visit(this, "binary operator", cast_variant<NODE_TYPES>(node->n_binary_operator->n_value));
            }
            else
            {
                g.visit(this, "expression", cast_variant<NODE_TYPES>(node->n_expression_lhs->n_value));
                g.push() << "rax\n";
                g.visit(this, "expression", cast_variant<NODE_TYPES>(node->n_expression_rhs->n_value));
                g.output() << "mov rbx, rax\n";
                g.pop() << "rax\n";
                g.visit(this, "binary operator", cast_variant<NODE_TYPES>(node->n_binary_operator->n_value));
            }
        }

        void operator()(const node_parenthesised_expression* node)
        {
            VERBOSE_OUT(2, "parenthesised expression\n", true);
            g.visit(this, "expression", cast_variant<NODE_TYPES>(node->n_expression->n_value));
        }

        void operator()(const node_binary_operator* node)
        {
            VERBOSE_OUT(2, "binary operator\n", true);
            g.visit(this, "binary operator", cast_variant<NODE_TYPES>(node->n_value));
        }

        void operator()(const node_parameter_pass* node)
        {
            VERBOSE_OUT(2, "parameter pass\n", true);
            g.visit(this, "parameter pass", cast_variant<NODE_TYPES>(node->n_value));
        }

        void operator()(const node_integer_literal* node)
        {
            VERBOSE_OUT(2, "integer literal\n", true);
            g.output() << "mov rax, " << node->value << '\n';
        }

        void operator()(const node_identifier* node)
        {
            VERBOSE_OUT(2, "identifier\n", true);

            auto object = g.get_object(node->value);
            if (!object) error_exit("Undefined object.");

            if (object->in_function())
            {
                g.output() << "mov rax, QWORD [" << object->get_address() << "]";
                VERBOSE_COMMENT(1, node->value, true) << '\n';
            }
            else
                assert(false && "identifier static object unimplemented.");
        }

        void operator()(const node_forward_slash& node)
        {
            VERBOSE_OUT(2, "/\n", true);
            g.output() << "div rbx\n";
        }

        void operator()(const node_percent& node)
        {
            VERBOSE_OUT(2, "%\n", true);
            g.output() << "div rbx\n";
            g.output() << "mov rax, rdx\n"; // div puts reg1 % reg2 in rdx
        }

        void operator()(const node_asterisk& node)
        {
            VERBOSE_OUT(2, "*\n", true);
            g.output() << "mul rbx\n";
        }

        void operator()(const node_plus& node)
        {
            VERBOSE_OUT(2, "+\n", true);
            g.output() << "add rax, rbx\n";
        }

        void operator()(const node_minus& node)
        {
            VERBOSE_OUT(2, "-\n", true);
            g.output() << "sub rax, rbx\n";
        }

        void operator()(const node_in& node)
        {
            assert(false && "in unimplemented.");
        }

        void operator()(const node_out& node)
        {
            assert(false && "out unimplemented.");
        }

        void operator()(const node_inout& node)
        {
            assert(false && "inout unimplemented.");
        }

        void operator()(const node_copy& node)
        {
            assert(false && "copy unimplemented.");
        }

        void operator()(const node_move& node)
        {
            assert(false && "move unimplemented.");
        }
    };

    std::string generator::operator()()
    {
        // Generate everything.
        generator_visitor(*this)(_root);
        generate_start();

        // Output data.
        for (auto& static_object : _static_objects)
            _output_data << static_object.name << ": dq 0\n";

        // Output text.
        for (auto& function : _functions)
            _output_text << '\n' << function.output.rdbuf();

        // Combine everything.
        std::stringstream output;
        if (_output_data.rdbuf()->in_avail())
            output << "section .data\n\n" << std::move(_output_data).rdbuf() << '\n';
        output << "section .text\n\n" << std::move(_output_start).rdbuf() << '\n' << std::move(_output_text).rdbuf() << '\n';
        return std::move(output).str();
    }

    std::stringstream& generator::output(bool indent)
    {
        if (indent)
        {
            IF_VERBOSE(2)
            {
                for (std::size_t i = 0; i < _indent_level; ++i)
                    *_output_current << "    ";
            }
            else
                *_output_current << "    ";
        }
        return *_output_current;
    }

    std::stringstream& generator::push()
    {
        ++_stack_pointer;
        output() << "push ";
        return *_output_current;
    }

    std::stringstream& generator::pop()
    {
        --_stack_pointer;
        output() << "pop ";
        return *_output_current;
    }

    std::stringstream& generator::output_label(std::string_view label)
    {
        IF_VERBOSE(2)
        {
            for (std::size_t i = 0; i < _indent_level - 1; ++i)
                *_output_current << "    ";
        }
        *_output_current << label << ':';
        return *_output_current;
    }

    void generator::begin_scope()
    {
        _scopes.push_back(get_current_function().objects.size());
    }

    void generator::end_scope()
    {
        auto& objects = get_current_function().objects;
        std::size_t pop_count = objects.size() - _scopes.back();
        if (pop_count > 0)
        {
            output() << "add rsp, " << (pop_count * elem_size) << '\n';
            _stack_pointer -= pop_count;
            objects.erase(objects.end() - pop_count, objects.end());
            // TODO: call destructors in reverse order.
        }
        _scopes.pop_back();
    }

    std::string generator::create_label(std::string_view short_name) noexcept
    {
        // As long as short_name is at most 5 characters:
        // an upper bound for the number of characters this label will take is:
        // "5 + ceil(log10(1 << 32)) == 15", which is exactly the capacity of
        // std::string's SSBO, meaning this will never allocate.
        return std::string(short_name) + std::to_string(_label_count++);
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

    void generator::visit(generator_visitor* visitor, std::string_view name, const nodes& variant)
    {
        IF_VERBOSE(2) output() << "; " << name << ": ";
        ++_indent_level;
        std::visit(*visitor, variant);
        --_indent_level;
    }

    auto generator::allocate_object(std::string_view name) -> object&
    {
        if (get_object(name))
            error_exit("Redefined variable.");
        auto& objects = get_objects();
        if (has_current_function())
        {
            auto& object = objects.emplace_back(name, -(1 + objects.size()));
            ++_stack_pointer;
            output() << "sub rsp, " << elem_size << '\n';
            return object;
        }
        else
            return _static_objects.emplace_back(name, 0);
    }

    void generator::deallocate_object()
    {
        assert(has_current_function());
        auto& function = get_current_function();
        function.objects.pop_back();
        --_stack_pointer;
        output() << "add rsp, " << elem_size << '\n';
    }

    auto generator::get_object(std::string_view name) -> object*
    {
        auto check = [=](const object& object) { return object.name == name; };
        object* object = nullptr;
        if (has_current_function())
        {
            auto& function = get_current_function();
                 if (auto it = std::ranges::find_if(function.parameters, check); it != function.parameters.end()) object = it.base();
            else if (auto it = std::ranges::find_if(function.return_values, check); it != function.return_values.end()) object = it.base();
            else if (auto it = std::ranges::find_if(function.objects, check); it != function.objects.end()) object = it.base();
        }
        if (!object)
            if (auto it = std::ranges::find_if(_static_objects, check); it != _static_objects.end()) object = it.base();
        return object;
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

    auto generator::get_objects() -> std::vector<object>&
    {
        return has_current_function() ? get_current_function().objects : _static_objects;
    }

    auto generator::get_current_function() -> function&
    {
        return _functions[_current_function_index];
    }

    bool generator::has_current_function() const noexcept
    {
        return 0 <= _current_function_index;
    }

    void generator::generate_start()
    {
        // Verify correct state.

        auto entry_point = get_function_from_name(get_input().entry_point);
        if (!entry_point) error_exit("Entry point not defined.");

        if (entry_point->return_values.size() > 1)
            error_exit("Ill-formed entry point. Incorrect return value count. Must be 0 or 1.");
        if (entry_point->parameters.size() != 2 && !entry_point->parameters.empty())
            error_exit("Ill-formed entry point. Incorrect parameter count. Must be 0 or 2.");

        // Generate start.

        _output_current = &_output_start;
        output(false) << "global _start\n_start:\n";

        if (!_static_objects.empty())
        {
            push() << "rdi\n";
            push() << "rsi\n";
            // TODO: call constructors of all static objects.
            pop() << "rsi\n";
            pop() << "rdi\n";
        }

        push() << '0'; VERBOSE_COMMENT(1, "status") << '\n';
        output() << "mov rbp, rsp\n";

        if (!entry_point->parameters.empty())
        {
            push() << "rdi"; VERBOSE_COMMENT(1, "argc") << '\n';
            push() << "rsi"; VERBOSE_COMMENT(1, "argv") << '\n';
        }

        output() << "call " << entry_point->signature << '\n';

        if (!_static_objects.empty())
        {
            push() << "rbp\n";
            // TODO: call destructors in reverse order of all static objects.
            pop() << "rbp\n";
        }

        output() << "mov rax, 60\n";
        output() << "mov rdi, [rbp]\n";
        output() << "syscall\n";
    }

    std::string generator::object::get_address() const
    {
        if (in_function())
        {
            std::string string = "rbp";
            if (std::ptrdiff_t rsp_offset = std::abs(stack_offset * elem_size))
            {
                string += ' ';
                string += (stack_offset > 0 ? '+' : '-');
                string += ' ';
                string += std::to_string(rsp_offset);
            }
            return string;
        }
        else
            return std::string(name);
    }
} // namespace shl
