#pragma once

#include "token.hpp"

namespace shl
{
    struct node_expression
    {
        token _integer_literal;
    };

    struct node_return
    {
        node_expression _expression;
    };
} // namespace shl
