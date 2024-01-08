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

    class generator
    {
    public:
        [[nodiscard]] explicit generator(node_program* root) : _root(root) {}

        [[nodiscard]] std::string operator()();

    private:
        struct object
        {
            // The name of the object.
            // When stack_offset is zero, this also serves as its address.
            std::string_view name;
            // The offset into the stack where this object resides.
            // If zero, this object is not on the stack.
            std::ptrdiff_t stack_offset = 0;

            // 0 (and 1 if rbp is pushed), are invalid stack offsets for functions.
            // 0 is guaranteed to be invalid, but not 1, so 0 is instead used to
            // say this object is not inside a function. This may mean it's static/
            // in the global scope, thread_local, or maybe something else.
            [[nodiscard]] constexpr bool in_function() const noexcept { return stack_offset != 0; }

            // Returns the address of the object.
            // If this object is in a function, returns "rsp+/-stack_offset".
            // Otherwise, returns the name of this object, which is a label
            // in the data section, i.e. an address to this object.
            [[nodiscard]] std::string get_address() const;
        };

        struct function
        {
            std::string_view name;
            std::string signature;
            std::vector<object> return_values;
            std::vector<object> parameters;
            std::vector<object> objects;
            std::stringstream output;
        };

    private:
        [[nodiscard]] std::stringstream& output(bool indent = true);
        [[nodiscard]] std::stringstream& push();
        [[nodiscard]] std::stringstream& pop();
        std::stringstream& output_label(std::string_view label);

        void begin_scope();
        void end_scope();

        // Creates a label string.
        // As long as short_name is at most 5 characters, this never allocates due to std::string's SSBO.
        [[nodiscard]] std::string create_label(std::string_view short_name = "label") noexcept;
        [[nodiscard]] std::string get_function_signature(const node_named_function* node);

        friend struct generator_visitor;
        void visit(generator_visitor* visitor, std::string_view name, const nodes& variant);

    private:
        object& allocate_object(std::string_view name);
        void deallocate_object();

        [[nodiscard]] object* get_object(std::string_view name);
        [[nodiscard]] function* get_function_from_name(std::string_view name);
        [[nodiscard]] function* get_function_from_signature(std::string_view name);
        [[nodiscard]] inline std::vector<object>& get_objects();
        [[nodiscard]] inline function& get_current_function();
        [[nodiscard]] inline bool has_current_function() const noexcept;

    private:
        void generate_start();

        // Move-appends source to destination, then returns destination for convenience.
        static std::stringstream& output_stream(std::stringstream& destination, std::stringstream& source);
        // Sets the current output to the new output and returns a reference to the old output.
        // Pass that reference back to this function to undo after you're done outputting.
        // The return value from the second call may be completely ignored.
        std::stringstream& exchange_current_output(std::stringstream& new_output);

        template <typename Func>
        void with_output(std::stringstream& output, const Func& func)
        {
            auto& output_backup = exchange_current_output(output);
            func();
            exchange_current_output(output_backup);
        }

    private:
        // Input

        node_program* _root;

        // Output

        // Code segments
        std::stringstream _output_data, _output_text;
        // Not code segments, just used to order assembly.
        std::stringstream _output_start, _output_static_construct, _output_static_destruct;
        // Points to different existing streams.
        std::stringstream* _output_current = &_output_text;
        // Number of indentation levels to indent the assembly by, in sets of 4 spaces.
        std::size_t _indent_level = 1;

        // Compilation state.

        // Increments when rsp decrements, and vice versa.
        std::size_t _stack_pointer = 0;
        // Index into _functions.
        // Used to determine which function is currently being generated, if any.
        std::ptrdiff_t _current_function_index = -1;
        // List of functions generated/being generated thus far.
        std::vector<function> _functions;
        // List of objects outside any function, aka static objects.
        std::vector<object> _static_objects;
        // Used to deallocate stack objects.
        std::vector<std::size_t> _scopes;
        // Used to generate unique labels for generic jumps, like if statements.
        std::uint32_t _label_count = 0;

    private:
        static constexpr std::ptrdiff_t elem_size = sizeof(std::uint64_t);
    };
} // namespace shl
