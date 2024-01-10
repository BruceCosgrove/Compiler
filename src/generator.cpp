#include "generator.hpp"
#include "error.hpp"
#include "util.hpp"
#include <algorithm>
#include <cassert>
#include <iostream> // DEBUG
#include <sstream>
#include <utility>

// NOTE: How to write to stdout:
// section .text
// string db "Hello world.", 0xA, 0
// ...
// mov rsi, string ; address of string
// mov rdx, 13     ; length of string
// mov rax, 1      ; command = write
// mov rdi, 1      ; select stdout
// syscall         ; call the kernel

#define VERBOSE_OUT(level, to_output, ...) \
        (IS_VERBOSE(level) ? static_cast<std::stringstream&>(__VA_OPT__(g.)output(false) << to_output) : (*__VA_OPT__(g.)_output.current))

#define VERBOSE_COMMENT(to_output, ...) VERBOSE_OUT(input::verbose_level::comments, " ; " << to_output __VA_OPT__(,) __VA_ARGS__)

namespace shl
{
    struct generator_visitor
    {
        generator& g; // context

        void operator()(const node_program* node)
        {
            VERBOSE_OUT(input::verbose_level::indentation, "program\n", true);
            for (auto node : node->declarations)
                g.visit(this, "program", cast_variant<NODE_TYPES>(node->n_value));
        }

        void operator()(const node_declaration* node)
        {
            VERBOSE_OUT(input::verbose_level::indentation, "declaration\n", true);
            g.visit(this, "declaration", cast_variant<NODE_TYPES>(node->n_value));
        }

        void operator()(const node_definition* node)
        {
            VERBOSE_OUT(input::verbose_level::indentation, "definition\n", true);
            g.visit(this, "definition", cast_variant<NODE_TYPES>(node->n_value));
        }

        void operator()(const node_declare_object* node)
        {
            VERBOSE_OUT(input::verbose_level::indentation, "declare object\n", true);
            if (g.has_current_function())
                g.create_object(node->n_name->value, false);
            else
                assert(false && "declaring static objects unimplemented.");
                // TODO: if next defining an object with the same name, it must be made initialized.
                // If it's not initialized by the end of generation, the program is ill-formed.
                // g.create_uninitialized(node->n_name->value);
        }

        void operator()(const node_define_object* node)
        {
            VERBOSE_OUT(input::verbose_level::indentation, "define object\n", true);

            if (g.has_current_function())
            {
                auto& object = g.create_object(node->n_name->value, false);
                g.visit(this, "expression", cast_variant<NODE_TYPES>(node->n_expression->n_value));
                g.output() << "mov [" << object.get_address() << "], rax";
                VERBOSE_COMMENT(node->n_name->value, true);
                g.output(false) << '\n';
            }
            else
            {
                // TODO: if the expression is constexpr, create an initialized object instead.
                auto& object = g.create_uninitialized(node->n_name->value);
                g.with_output(g._output.uninitialized_static_construct, [&]
                {
                    g.visit(this, "expression", cast_variant<NODE_TYPES>(node->n_expression->n_value));
                    g.output() << "mov [" << object.get_address() << "], rax\n";
                });
            }
        }

        void operator()(const node_function* node)
        {
            VERBOSE_OUT(input::verbose_level::indentation, "function\n", true);
            assert(false && "unnamed function unimplemented.");
        }

        void operator()(const node_named_function* node)
        {
            VERBOSE_OUT(input::verbose_level::indentation, "named function\n", true);

            std::stringstream s_namespace;
            for (std::string_view nested_signature : g._nested_function_signatures)
                s_namespace << nested_signature << ".."; // The .. replaces :: (namespace resolution operator).
            std::string signature = g.create_function_signature(node);
            if (g.get_function_from_signature(signature)) error_exit("Generator", "Redefined function");

            auto& functions = g.has_current_function() ? g.get_current_function().nested_functions : g._functions;
            auto& function = functions.emplace_back(node->n_name->value, std::move(signature), std::move(s_namespace).str());
            g._nested_function_signatures.push_back(function.signature);

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

            g.with_output(function.output, [&]
            {
                // TODO: dont allocate this
                g.output_label(function.namespace_ + function.signature) << '\n';
                g.output() << "push rbp\n";
                g.output() << "mov rbp, rsp\n";
                g.visit(this, "statement", cast_variant<NODE_TYPES>(node->n_function->n_statement->n_value));
                g.output() << "pop rbp\n";
                g.output() << "ret\n";
            });

            g._nested_function_signatures.pop_back();
        }

        void operator()(const node_parameter* node)
        {
            VERBOSE_OUT(input::verbose_level::indentation, "parameter\n", true);
            assert(false && "parameter unimplemented.");
        }

        void operator()(const node_scope* node)
        {
            VERBOSE_OUT(input::verbose_level::indentation, "scope\n", true);
            g.begin_scope();
            for (auto n_scoped_statement : node->scoped_statements)
                g.visit(this, "scoped statement", cast_variant<NODE_TYPES>(n_scoped_statement->n_value));
            g.end_scope();
        }

        void operator()(const node_statement* node)
        {
            VERBOSE_OUT(input::verbose_level::indentation, "statement\n", true);
            g.visit(this, "statement", cast_variant<NODE_TYPES>(node->n_value));
        }

        void operator()(const node_scoped_statement* node)
        {
            VERBOSE_OUT(input::verbose_level::indentation, "scoped statement\n", true);
            g.visit(this, "scoped statement", cast_variant<NODE_TYPES>(node->n_value));
        }

        void operator()(const node_expression* node)
        {
            VERBOSE_OUT(input::verbose_level::indentation, "expression\n", true);
            g.visit(this, "expression", cast_variant<NODE_TYPES>(node->n_value));
        }

        void operator()(const node_term* node)
        {
            VERBOSE_OUT(input::verbose_level::indentation, "term\n", true);
            g.visit(this, "term", cast_variant<NODE_TYPES>(node->n_value));
        }

        void operator()(const node_return* node)
        {
            VERBOSE_OUT(input::verbose_level::indentation, "return\n", true);
            g.output() << "pop rbp\n";
            g.output() << "ret\n";
        }

        void operator()(const node_if* node)
        {
            VERBOSE_OUT(input::verbose_level::indentation, "if\n", true);
            g.visit(this, "expression", cast_variant<NODE_TYPES>(node->n_expression->n_value));
            std::string label_end_if = g.create_label();
            g.output() << "test rax, rax\n";
            g.output() << "jz " << label_end_if << '\n';
            g.visit(this, "statement", cast_variant<NODE_TYPES>(node->n_statement->n_value));
            g.output_label(label_end_if);
            VERBOSE_COMMENT("endif", true) << '\n';
        }

        void operator()(const node_reassign* node)
        {
            VERBOSE_OUT(input::verbose_level::indentation, "reassign\n", true);

            auto object = g.get_object(node->n_identifier->value);
            if (!object) ERROR_EXIT("Generator", "Undefined object \"" << node->n_identifier->value << '"');

            g.visit(this, "expression", cast_variant<NODE_TYPES>(node->n_expression->n_value));
            g.output() << "mov [" << object->get_address() << "], rax";
            if (!object->is_static())
                VERBOSE_COMMENT(node->n_identifier->value, true);
            g.output(false) << '\n';
        }

        void operator()(const node_scoped_if* node)
        {
            VERBOSE_OUT(input::verbose_level::indentation, "scoped if\n", true);

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
                    VERBOSE_COMMENT((node->ifs[i]->n_expression ? "elif" : "else"), true) << '\n';
                    label_next_if = i + 1 < node->ifs.size() ? g.create_label() : label_end_if;
                }
            }
            while (i < node->ifs.size());

            g.output_label(label_end_if);
            VERBOSE_COMMENT("endif", true) << '\n';
        }

        void operator()(const node_binary_expression* node)
        {
            VERBOSE_OUT(input::verbose_level::indentation, "binary expression\n", true);

            bool expand_lhs = std::holds_alternative<node_binary_expression*>(node->n_expression_lhs->n_value) ||
                std::holds_alternative<node_parenthesised_expression*>(std::get<node_term*>(node->n_expression_lhs->n_value)->n_value);
            bool expand_rhs = std::holds_alternative<node_binary_expression*>(node->n_expression_rhs->n_value) ||
                std::holds_alternative<node_parenthesised_expression*>(std::get<node_term*>(node->n_expression_rhs->n_value)->n_value);

            if (!expand_lhs && !expand_rhs) // both are leaves
            {
                g.visit(this, "expression", cast_variant<NODE_TYPES>(node->n_expression_lhs->n_value));
                g.output() << "mov rbx, rax\n";
                g.visit(this, "expression", cast_variant<NODE_TYPES>(node->n_expression_rhs->n_value));
                g.visit(this, "binary operator", cast_variant<NODE_TYPES>(node->n_binary_operator->n_value));
            }
            else if (!expand_lhs) // lhs is a leaf, but rhs is not
            {
                g.visit(this, "expression", cast_variant<NODE_TYPES>(node->n_expression_rhs->n_value)); // compute rhs first
                g.output() << "push rax\n";
                g.visit(this, "expression", cast_variant<NODE_TYPES>(node->n_expression_lhs->n_value));
                g.output() << "pop rbx\n";
                g.visit(this, "binary operator", cast_variant<NODE_TYPES>(node->n_binary_operator->n_value));
            }
            else
            {
                g.visit(this, "expression", cast_variant<NODE_TYPES>(node->n_expression_lhs->n_value));
                g.output() << "push rax\n";
                g.visit(this, "expression", cast_variant<NODE_TYPES>(node->n_expression_rhs->n_value));
                g.output() << "mov rbx, rax\n";
                g.output() << "pop rax\n";
                g.visit(this, "binary operator", cast_variant<NODE_TYPES>(node->n_binary_operator->n_value));
            }
        }

        void operator()(const node_parenthesised_expression* node)
        {
            VERBOSE_OUT(input::verbose_level::indentation, "parenthesised expression\n", true);
            g.visit(this, "expression", cast_variant<NODE_TYPES>(node->n_expression->n_value));
        }

        void operator()(const node_binary_operator* node)
        {
            VERBOSE_OUT(input::verbose_level::indentation, "binary operator\n", true);
            g.visit(this, "binary operator", cast_variant<NODE_TYPES>(node->n_value));
        }

        void operator()(const node_parameter_pass* node)
        {
            VERBOSE_OUT(input::verbose_level::indentation, "parameter pass\n", true);
            g.visit(this, "parameter pass", cast_variant<NODE_TYPES>(node->n_value));
        }

        void operator()(const node_integer_literal* node)
        {
            VERBOSE_OUT(input::verbose_level::indentation, "integer literal\n", true);
            g.output() << "mov rax, " << node->value << '\n';
        }

        void operator()(const node_identifier* node)
        {
            VERBOSE_OUT(input::verbose_level::indentation, "identifier\n", true);

            auto object = g.get_object(node->value);
            if (!object) error_exit("Generator", "Undeclared identifier");

            g.output() << "mov rax, QWORD [" << object->get_address() << "]";
            if (!object->is_static())
                VERBOSE_COMMENT(node->value, true);
            g.output(false) << '\n';
        }

        void operator()(const node_forward_slash& node)
        {
            VERBOSE_OUT(input::verbose_level::indentation, "/\n", true);
            g.output() << "div rbx\n";
        }

        void operator()(const node_percent& node)
        {
            VERBOSE_OUT(input::verbose_level::indentation, "%\n", true);
            g.output() << "div rbx\n";
            g.output() << "mov rax, rdx\n"; // div puts reg1 % reg2 in rdx
        }

        void operator()(const node_asterisk& node)
        {
            VERBOSE_OUT(input::verbose_level::indentation, "*\n", true);
            g.output() << "mul rbx\n";
        }

        void operator()(const node_plus& node)
        {
            VERBOSE_OUT(input::verbose_level::indentation, "+\n", true);
            g.output() << "add rax, rbx\n";
        }

        void operator()(const node_minus& node)
        {
            VERBOSE_OUT(input::verbose_level::indentation, "-\n", true);
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
        generator_visitor visitor(*this);
        visit(&visitor, "program", nodes(_root));
        generate_start();

        // Output bss.
        _output.bss << "section .bss\n\n";
        IF_VERBOSE(input::verbose_level::comments) _output.bss << "; Allocate UNinitialized global/static objects.\n";
        for (auto& object : _uninitialized_static_objects)
            _output.bss << object.get_address() << ": resq 1\n";
        for (auto& function : _functions)
            for (auto& object : function.static_objects)
                _output.bss << object.get_address() << ": resq 1\n";

        // Output data.
        _output.data << "section .data\n\n";
        IF_VERBOSE(input::verbose_level::comments) _output.data << "; Allocate/define initialized global/static objects.\n";
        output_stream(_output.data, _output.initialized_static);

        // Output text.
        _output.text << "section .text\n\n";
        IF_VERBOSE(input::verbose_level::comments) _output.text << "; Allocate/define constant objects.\n";
        output_stream(_output.text, _output.constants) << '\n';

        IF_VERBOSE(input::verbose_level::comments) _output.text << "; Define provided functions.\n";
        for (auto& function : _functions)
            generate_function(function);
        IF_VERBOSE(input::verbose_level::comments) _output.text << "; Define the pre-entrypoint.\n";
        output_stream(_output.text, _output._start);

        // Combine everything and return the final assembly.
        std::stringstream output;
        output_stream(output, _output.bss) << '\n';
        output_stream(output, _output.data) << '\n';
        output_stream(output, _output.text);
        return std::move(output).str();
    }

    std::stringstream& generator::output(bool indent)
    {
        if (indent)
        {
            IF_VERBOSE(input::verbose_level::indentation)
            {
                for (std::size_t i = 0; i < _output.indent_level; ++i)
                    *_output.current << "    ";
            }
            else
                *_output.current << "    ";
        }
        return *_output.current;
    }

    std::stringstream& generator::output_label(std::string_view label)
    {
        IF_VERBOSE(input::verbose_level::indentation)
        {
            for (std::size_t i = 0; i < _output.indent_level - 1; ++i)
                *_output.current << "    ";
        }
        *_output.current << label << ':';
        return *_output.current;
    }

    std::stringstream& generator::exchange_current_output(std::stringstream& new_output)
    {
        return *std::exchange(_output.current, &new_output);
    }

    std::stringstream& generator::output_stream(std::stringstream& destination, std::stringstream& source)
    {
        if (source.rdbuf()->in_avail() > 0)
            destination << std::move(source).rdbuf();
        return destination;
    }

    void generator::begin_scope()
    {
        _scopes.push_back(get_current_function().objects.size());
    }

    void generator::end_scope()
    {
        auto& objects = get_current_function().objects;
        if (std::size_t pop_count = objects.size() - _scopes.back())
        {
            output() << "add rsp, " << (pop_count * elem_size) << '\n';
            objects.erase(objects.end() - pop_count, objects.end());
        }
        _scopes.pop_back();
    }

    auto generator::create_uninitialized(std::string_view name) -> object&
    {
        if (get_object(name)) error_exit("Generator", "Redefined object");
        return _uninitialized_static_objects.emplace_back(name, 0);
    }

    auto generator::create_initialized(std::string_view name) -> object&
    {
        if (get_object(name)) error_exit("Generator", "Redefined object");
        return _initialized_static_objects.emplace_back(name, 0);
    }

    auto generator::create_constant(std::string_view name) -> object&
    {
        if (get_object(name)) error_exit("Generator", "Redefined object");
        return _constant_objects.emplace_back(name, 0);
    }

    auto generator::create_object(std::string_view name, bool is_static) -> object&
    {
        if (get_object(name)) error_exit("Generator", "Redefined object");
        assert(has_current_function());

        if (!is_static)
        {
            auto& objects = get_current_function().objects;
            auto& object = objects.emplace_back(name, -(1 + objects.size())); // +1 for push rbp
            output() << "sub rsp, " << elem_size << '\n';
            return object;
        }
        else
        {
            auto& function = get_current_function();
            std::string object_address = function.signature;
            object_address += "::";
            object_address += name;
            return function.static_objects.emplace_back(object_address, 0);
        }
    }

    auto generator::get_object(std::string_view name) -> object*
    {
        auto check = [=](const object& object) { return object.name == name; };
        object* object_ = nullptr;
        if (has_current_function())
        {
            auto& function = get_current_function();
                 if (auto it = std::ranges::find_if(function.parameters, check); it != function.parameters.end()) object_ = it.base();
            else if (auto it = std::ranges::find_if(function.return_values, check); it != function.return_values.end()) object_ = it.base();
            else if (auto it = std::ranges::find_if(function.objects, check); it != function.objects.end()) object_ = it.base();
            else if (auto it = std::ranges::find_if(function.static_objects, check); it != function.static_objects.end()) object_ = it.base();
        }
        if (!object_)
        {
                 if (auto it = std::ranges::find_if(_uninitialized_static_objects, check); it != _uninitialized_static_objects.end()) object_ = it.base();
            else if (auto it = std::ranges::find_if(_initialized_static_objects, check); it != _initialized_static_objects.end()) object_ = it.base();
        }
        return object_;
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
        function* function = nullptr;
        if (has_current_function())
        {
            auto& functions = get_current_function().nested_functions;
            if (check(get_current_function())) function = &get_current_function();
            else if (auto it = std::ranges::find_if(functions, check); it != functions.end()) function = it.base();
        }
        if (!function)
            if (auto it = std::ranges::find_if(_functions, check); it != _functions.end()) function = it.base();
        return function;
    }

    auto generator::get_current_function() -> function&
    {
        assert(has_current_function());
        function* function = &_functions.back();
        while (!function->nested_functions.empty())
            function = &function->nested_functions.back();
        return *function;
    }

    bool generator::has_current_function() const noexcept
    {
        return !_nested_function_signatures.empty();
    }

    std::string generator::create_label(std::string_view short_name) noexcept
    {
        // As long as short_name is at most 5 characters:
        // an upper bound for the number of characters this label will take is:
        // "5 + ceil(log10(1 << 32)) == 15", which is exactly the capacity of
        // std::string's SSBO, meaning this will never allocate.
        static std::uint32_t _label_count = 0;
        return std::string(short_name) + std::to_string(_label_count++);
    }

    std::string generator::create_function_signature(const node_named_function* node)
    {
        std::stringstream s_signaure;
        s_signaure << node->n_name->value;

        s_signaure << '_';
        for (auto n_return_value : node->n_function->return_values)
            s_signaure << '_' << n_return_value->n_name->value; // TODO: When types are implemented, change "n_name" to "n_type".

        s_signaure << '_';
        for (auto n_parameter : node->n_function->parameters)
            s_signaure << '_' << n_parameter->n_name->value; // TODO: When types are implemented, change "n_name" to "n_type".

        // Change asterisks (from pointer types) to periods to appease assembler.
        std::string signature = s_signaure.str();
        for (char& c : signature)
            if (c == '*')
                c = '.';
        return signature;
    }

    void generator::generate_start()
    {
        // Verify correct state.

        auto entry_point = get_function_from_name(get_input().entry_point);
        if (!entry_point) return;

        if (entry_point->return_values.size() > 1)
            error_exit("Generator", "Ill-formed entry point. Incorrect return value count. Must be 0 or 1");
        if (entry_point->parameters.size() != 2 && !entry_point->parameters.empty())
            error_exit("Generator", "Ill-formed entry point. Incorrect parameter count. Must be 0 or 2");

        // Generate start.

        auto& output_backup = exchange_current_output(_output._start);
        output(false) << "global _start\n_start:\n";

        if (!_uninitialized_static_objects.empty())
        {
            IF_VERBOSE(input::verbose_level::comments) output() << "; Construct static objects.\n";
            output_stream(*_output.current, _output.uninitialized_static_construct);
        }

        IF_VERBOSE(input::verbose_level::comments) output() << "; Call entrypoint.\n";
        output() << "push 0"; VERBOSE_COMMENT("status") << '\n';
        output() << "mov rbp, rsp\n";

        if (!entry_point->parameters.empty())
        {
            output() << "push rdi"; VERBOSE_COMMENT("argc") << '\n';
            output() << "push rsi"; VERBOSE_COMMENT("argv") << '\n';
        }

        output() << "call " << entry_point->signature << '\n';

        if (!_uninitialized_static_objects.empty())
        {
            IF_VERBOSE(input::verbose_level::comments) output() << "; Destruct static objects.\n";
            output_stream(*_output.current, _output.uninitialized_static_destruct);
        }

        IF_VERBOSE(input::verbose_level::comments) output() << "; Exit with return code [rbp].\n";
        output() << "mov rax, 60\n";
        output() << "mov rdi, [rbp]\n";
        output() << "syscall\n";

        exchange_current_output(output_backup);
    }

    void generator::generate_function(function& function)
    {
        output_stream(_output.text, function.output) << '\n';
        for (auto& nested_function : function.nested_functions)
            generate_function(nested_function);
    }

    std::string generator::object::get_address() const
    {
        std::string address;
        if (!is_static())
        {
            address = "rbp";
            if (std::ptrdiff_t rsp_offset = std::abs(stack_offset * elem_size))
            {
                address += ' ';
                address += (stack_offset > 0 ? '+' : '-');
                address += ' ';
                address += std::to_string(rsp_offset);
            }
        }
        else
        {
            address = name;
            address += '_';
        }
        return address;
    }
} // namespace shl
