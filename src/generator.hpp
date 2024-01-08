#pragma once

#include "input.hpp"
#include "token_nodes.hpp"
#include <cassert> // DEBUG
#include <sstream>
#include <string>
#include <vector>

namespace shl
{
    struct generator_visitor; // implementation

    class generator // Making this a consumer turned out to be too much overhead.
    {
    public:
        [[nodiscard]] explicit generator(node_program* root) : _root(root) {}

        [[nodiscard]] std::string operator()();

    private:
        [[nodiscard]] std::stringstream& output(bool indent = true);
        [[nodiscard]] std::stringstream& push();
        [[nodiscard]] std::stringstream& pop();
        std::stringstream& output_label(std::string_view label);

        void begin_scope();
        void end_scope();

        void generate_start();

        // Creates a label string.
        // As long as short_name is at most 5 characters, this never allocates due to std::string's SSBO.
        [[nodiscard]] std::string create_label(std::string_view short_name = "label") noexcept;
        [[nodiscard]] std::string get_function_signature(const node_named_function* node);

        void visit(generator_visitor* visitor, std::string_view name, const nodes& variant);
    private:
        struct variable
        {
            // The name of the variable.
            // When stack_offset is zero, this also serves as its address.
            std::string_view name;
            std::ptrdiff_t stack_offset = 0;

            // 0 (and 1 if rbp is pushed), are invalid stack offsets for functions.
            // 0 is guaranteed to be invalid, but not 1, so 0 is instead used to
            // say this variable is not inside a function. This may mean it's static/
            // in the global scope, thread_local, or maybe something else.
            [[nodiscard]] constexpr bool in_function() const noexcept { return stack_offset != 0; }

            // Returns the address of the variable.
            // If this variable is in a function, returns "rsp+/-stack_offset".
            // Otherwise, returns the name of this variable, which is a label
            // in the data section, i.e. an address to this variable.
            [[nodiscard]] std::string get_address() const;
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

        friend struct generator_visitor;

    private:
        variable& allocate_variable(std::string_view name);
        void deallocate_variable();

        [[nodiscard]] variable* get_variable(std::string_view name);
        [[nodiscard]] function* get_function_from_name(std::string_view name);
        [[nodiscard]] function* get_function_from_signature(std::string_view name);
        [[nodiscard]] inline std::vector<variable>& get_variables();
        [[nodiscard]] inline function& get_current_function();
        [[nodiscard]] inline bool has_current_function() const noexcept;

        // Input

        node_program* _root;

        // Output

        std::stringstream _output_data;
        std::stringstream _output_text;
        std::stringstream _output_start; // Not a code segment, just used to order assembly.
        std::stringstream* _output_current; // Points to different existing streams.
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
