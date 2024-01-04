#pragma once

#include "token.hpp"
#include <variant>
#include <vector>

namespace shl
{
    struct node_integer_literal
    {
        token _token;
    };

    struct node_identifier
    {
        token _token;
    };

    using node_expression = std::variant<node_integer_literal, node_identifier>;

    struct node_return
    {
        node_expression _expression;
    };

    struct node_declare_identifier
    {
        node_identifier _identifier;
        node_expression _expression;
    };

    using node_statement = std::variant<node_return, node_declare_identifier>;

    struct node_program
    {
        std::vector<node_statement> _statements;
    };
} // namespace shl
