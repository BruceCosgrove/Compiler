#pragma once

#include "middle/ast.hpp"
#include "back/ir_code.hpp"

namespace shl
{
    class ir_code_generator
    {
    public:
        [[nodiscard]] explicit ir_code_generator(node_program* ast) : _ast(ast) {}

        [[nodiscard]] ir_code operator()();
    private:
        node_program* _ast;
    };
} // namespace shl
