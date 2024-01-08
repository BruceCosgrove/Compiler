#pragma once

#include "input.hpp"
#include "token_nodes.hpp"
#include <cassert> // DEBUG
#include <sstream>
#include <string>
#include <vector>

namespace shl
{
    class generator // Making this a consumer turned out to be too much overhead.
    {
    public:
        [[nodiscard]] explicit generator(node_program* root) : _root(root) {}

        [[nodiscard]] std::string operator()();

    private:
        // Correspondence: Cased grammar rule.

        void generate_program();
        void generate_declaration(std::stringstream& out, const node_declaration* node);
        void generate_definition(std::stringstream& out, const node_definition* node);
        void generate_scope(std::stringstream& out, const node_scope* node);
        void generate_statement(std::stringstream& out, const node_statement* node);
        void generate_scoped_statement(std::stringstream& out, const node_scoped_statement* node);
        void generate_expression(std::stringstream& out, const node_expression* node);
        void generate_term(std::stringstream& out, const node_term* node);
        void generate_binary_operator(std::stringstream& out, const node_binary_operator* node);

        void generate_if(std::stringstream& out, const node_if* node);
        void generate_scoped_if(std::stringstream& out, const node_scoped_if* node);
    private:
        [[nodiscard]] std::stringstream& output(std::stringstream& out, bool indent = true);
        [[nodiscard]] std::stringstream& push(std::stringstream& out);
        [[nodiscard]] std::stringstream& pop(std::stringstream& out);
        std::stringstream& output_label(std::stringstream& out, std::string_view label);

        [[nodiscard]] std::string stack_frame_offset(std::ptrdiff_t offset);

        void begin_scope();
        void end_scope(std::stringstream& out);

        void generate_start();

        // Creates a label string.
        // As long as short_name is at most 5 characters, this never allocates due to std::string's SSBO.
        [[nodiscard]] std::string create_label(std::string_view short_name = "label") noexcept;
        [[nodiscard]] std::string get_function_signature(const node_named_function* node);

        template <typename visitor, typename... types, typename... Args>
        void visit(std::stringstream& out, std::string_view name, const std::variant<types...>& variant, Args&&... args)
        {
            IF_VERBOSE(2) output(out) << "; " << name << ": ";
            ++_indent_level;
            std::visit(visitor(*this, out, std::forward<Args>(args)...), variant);
            --_indent_level;
        }
    private:
        struct variable
        {
            std::string_view name;
            std::ptrdiff_t stack_offset = 0;

            // 0 (and 1 if rbp is pushed), are invalid stack offsets for functions.
            // 0 is guaranteed to be invalid, but not 1, so 0 is instead used to
            // say this variable is not inside a function. This may mean it's static/
            // in the global scope, thread_local, or maybe something else.
            [[nodiscard]] constexpr bool in_function() const noexcept { return stack_offset != 0; }
        };

        struct function
        {
            std::string_view name;
            std::string signature;
            std::vector<variable> return_values;
            std::vector<variable> parameters;
            std::vector<variable> variables;
            std::stringstream output;
        };

    private:
        variable& allocate_variable(std::stringstream& out, std::string_view name);
        void deallocate_variable(std::stringstream& out);

        function& create_function(const node_named_function* node);

        [[nodiscard]] variable* get_variable(std::string_view name);
        [[nodiscard]] function* get_function_from_name(std::string_view name);
        [[nodiscard]] function* get_function_from_signature(std::string_view name);
        [[nodiscard]] inline std::vector<variable>& get_variables();
        [[nodiscard]] inline function& get_current_function();
        [[nodiscard]] inline bool has_current_function() const noexcept;

        // Input

        const node_program* _root;

        // Output

        std::stringstream _output_data;
        std::stringstream _output_text;
        std::stringstream _output_start; // Not a code segment, just used to order assembly.s
        // Number of indentation levels to indent the assembly by, in sets of 4 spaces.
        std::size_t _indent_level = 1;

        // Compilation state.

        // Increments when rsp decrements, and vice versa.
        std::size_t _stack_pointer = 0;
        std::ptrdiff_t _current_function_index = -1;
        std::vector<function> _functions;
        std::vector<variable> _static_variables;
        std::vector<std::size_t> _scopes;
        std::uint32_t _label_count = 0;

    private:
        static constexpr std::ptrdiff_t elem_size = sizeof(std::uint64_t);
    };
} // namespace shl
