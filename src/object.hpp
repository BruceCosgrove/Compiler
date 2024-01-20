#pragma once

#include <string>

namespace shl
{
    // A variable or constant that can be referred to in a program.
    // It may exist in the program's stack during runtime, in constant memory, or unitialized memory.
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
        [[nodiscard]] bool is_static() const noexcept;

        // Returns the address of the object.
        // If this object is in a function, returns "rsp+/-stack_offset".
        // Otherwise, returns the name of this object with an appended underscore,
        // which is a label in the data section, i.e. an address to this object.
        [[nodiscard]] std::string get_address() const;
    };
} // namespace shl
