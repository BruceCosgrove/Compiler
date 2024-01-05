#pragma once

#include "token.hpp"
#include <variant>
#include <vector>

namespace shl
{
    // Declare all the nodes.

    // Each of these correspond to a grammar rule.

    struct node_program;
    struct node_statement;
    struct node_expression;
    struct node_term;
    struct node_binary_expression;
    struct node_integer_literal;
    struct node_identifier;
    struct node_binary_operator;

    // Each of these correspond to a grammar subrule.

    struct node_return;
    struct node_declare_identifier;

    // Each of these are operators.

    struct node_asterisk {};
    struct node_forward_slash {};
    struct node_percent {};
    struct node_plus {};
    struct node_minus {};


    // Define all the nodes, again in order.

    struct node_program
    {
        std::vector<node_statement*> _statements;
    };

    struct node_statement
    {
        std::variant<node_return*, node_declare_identifier*> _statement;
    };

    struct node_expression
    {
        std::variant<node_term*, node_binary_expression*> _expression;
    };

    struct node_term
    {
        std::variant<node_integer_literal*, node_identifier*> _term;
    };

    struct node_binary_expression
    {
        node_expression* _expression_left;
        node_binary_operator* _binary_operator;
        node_expression* _expression_right;
    };

    struct node_integer_literal
    {
        token _token;
    };

    struct node_identifier
    {
        token _token;
    };

    struct node_binary_operator
    {
        // TODO: other operators (in the right order).
        std::variant<node_plus*> _binary_operator;
    };

    struct node_return
    {
        node_expression* _expression;
    };

    struct node_declare_identifier
    {
        node_identifier* _identifier;
        node_expression* _expression;
    };
} // namespace shl
