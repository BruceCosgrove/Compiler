#pragma once

#include "common/ranged_enum.hpp"
#include <string_view>
#include <variant>
#include <vector>

namespace shl
{
    // Operations.
    DEFINE_RANGED_ENUM(ir_op,
        (
            // boolean

            gt, // >
            ge, // >=
            lt, // <
            le, // <=
            eq, // ==
            ne, // !=

            // arithmetic

            div, // /
            mod, // %
            mul, // *
            add, // +
            sub  // -
        ),
        // Ranges
        (
            (boolean, gt, ne + 1),
            (arithmetic, div, sub + 1)
        )
    );

    // Declare all the line types.

    struct ir_assign;          // a = b
    struct ir_assign_op;       // a = b op c
    struct ir_assign_indirect; // a = [b]
    struct ir_indirect_assign; // [b] = a
    struct ir_if;              // if a op b goto L# // where op in ir_op::boolean
    struct ir_goto;            // goto L#
    struct ir_label;           // L#:
    // struct ir_call;            // func(r1, ..., rN; a1, ..., aN)


    // All line types.
    #define IR_TYPES \
        ir_assign*, \
        ir_assign_op*, \
        ir_assign_indirect*, \
        ir_indirect_assign*, \
        ir_if*, \
        ir_goto*, \
        ir_label*

    // Variant of all line types.
    using ir_line = std::variant<IR_TYPES>;


    // Define all the line types in the same order.

    struct ir_assign
    {
        std::size_t dst;
        std::size_t src;
    };

    struct ir_assign_op
    {
        std::size_t dst;
        std::size_t lhs;
        std::size_t rhs;
        ir_op op;
    };

    struct ir_assign_indirect
    {
        std::size_t dst;
        std::size_t src;
    };

    struct ir_indirect_assign
    {
        std::size_t dst;
        std::size_t src;
    };

    struct ir_if
    {
        std::size_t lhs;
        std::size_t rhs;
        ir_op op;
    };

    struct ir_goto
    {
        std::size_t lbl;
    };

    struct ir_label
    {
        std::size_t lbl;
    };
} // namespace shl
