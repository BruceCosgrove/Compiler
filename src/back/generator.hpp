#pragma once

#include "input.hpp"
#include "middle/ast.hpp"
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
            // When stack_offset is zero, this also serves as its address (with an underscore appended).
            std::string_view name;
            // The offset into the stack where this object resides.
            // If zero, this object is not on the stack.
            std::ptrdiff_t stack_offset = 0;

            // 0 (and 1 if rbp is pushed), are invalid stack offsets for functions.
            // 0 is guaranteed to be invalid, but not 1, so 0 is instead used to
            // say this object is not inside a function. This may mean it's static/
            // in the global scope, thread_local, or maybe something else.
            [[nodiscard]] constexpr bool is_static() const noexcept { return stack_offset == 0; }

            // Returns the address of the object.
            // If this object is in a function, returns "rsp+/-stack_offset".
            // Otherwise, returns the name of this object with an appended underscore,
            // which is a label in the data section, i.e. an address to this object.
            [[nodiscard]] std::string get_address() const;
        };

        struct function
        {
            std::string_view name;
            std::string signature;
            std::string namespace_;
            std::vector<object> return_values;
            std::vector<object> parameters;
            std::vector<object> objects;
            std::vector<object> static_objects;
            std::vector<function> nested_functions;
            std::stringstream output;
        };

    private:
        // Optionally outputs appropriate indentation for instructions to the current output stream.
        // Returns the current output stream for convenience.
        [[nodiscard]] std::stringstream& output(bool indent = true);

        // Optionally outputs appropriate indentation for the label to the current output stream,
        // followed by the label itself and a colon. Returns the current output stream for convenience.
        std::stringstream& output_label(std::string_view label);

        // Sets the current output to the new output and returns a reference to the old output.
        // Pass that reference back to this function to undo after you're done outputting.
        // The return value from the second call may be completely ignored.
        std::stringstream& exchange_current_output(std::stringstream& new_output);

        // Move-appends source to destination. Returns destination for convenience.
        static std::stringstream& output_stream(std::stringstream& destination, std::stringstream& source);

        // Calls a function with the specific output stream.
        template <typename Func>
        void with_output(std::stringstream& output, const Func& func)
        {
            auto& output_backup = exchange_current_output(output);
            func();
            exchange_current_output(output_backup);
        }

    private:
        // Selects which generation function to call next.
        template <class Visitor>
        void visit(Visitor* visitor, std::string_view name, const nodes& variant)
        {
            IF_VERBOSE(input::verbose_level::indentation) output() << "; " << name << ": ";
            ++_output.indent_level;
            std::visit(*visitor, variant);
            --_output.indent_level;
        }

        // Allocates a new scope frame.
        void begin_scope();

        // Deallocates the top scope frame and any variables local in it.
        void end_scope();

    private:
        // Creates a global object in bss.
        object& create_uninitialized(std::string_view name);

        // Creates a global object in data.
        object& create_initialized(std::string_view name);

        // Creates a global constant in text.
        object& create_constant(std::string_view name);

        // Creates a local object the current function's stack frame (also in text).
        object& create_object(std::string_view name, bool is_static);

        [[nodiscard]] object* get_object(std::string_view name);
        [[nodiscard]] function* get_function_from_name(std::string_view name);
        [[nodiscard]] function* get_function_from_signature(std::string_view name);
        [[nodiscard]] inline function& get_current_function();
        [[nodiscard]] inline bool has_current_function() const noexcept;

        // Creates a label string.
        // As long as short_name is at most 5 characters, this never allocates due to std::string's SSBO.
        [[nodiscard]] static std::string create_label(std::string_view short_name = "label") noexcept;

        // Creates a function's signature. This likely allocates.
        [[nodiscard]] std::string create_function_signature(const node_named_function* node);

    private:
        // Generates the pre-entrypoint function if main is defined.
        void generate_start();
        // Generates a function and its nested functions.
        void generate_function(function& function);

        friend struct generator_visitor;

    private:
        // Input

        node_program* _root;

        // Output

        struct
        {
            // Code segments
            std::stringstream bss, data, text;
            // Not code segments, just used to order assembly.
            std::stringstream _start;
            std::stringstream uninitialized_static_construct;
            std::stringstream uninitialized_static_destruct;
            std::stringstream initialized_static;
            std::stringstream constants;
            // Points to different existing streams.
            std::stringstream* current = nullptr;
            // Number of indentation levels to indent the assembly by, in sets of 4 spaces.
            std::uint32_t indent_level = 1;
        } _output;

        // Compilation state.

        // List of nested function signatures.
        std::vector<std::string_view> _nested_function_signatures;
        // List of functions generated/being generated thus far.
        std::vector<function> _functions;
        // List of uninitialized aka static objects.
        std::vector<object> _uninitialized_static_objects;
        // List of initialized static static objects.
        std::vector<object> _initialized_static_objects;
        // List of constant objects.
        std::vector<object> _constant_objects;
        // List of stack offsets.
        // Used to deallocate stack objects.
        std::vector<std::size_t> _scopes;

    private:
        static constexpr std::ptrdiff_t elem_size = sizeof(std::uint64_t);
    };
} // namespace shl
