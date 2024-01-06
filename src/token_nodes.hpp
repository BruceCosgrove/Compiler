#pragma once

#include "token.hpp"
#include <variant>
#include <vector>

namespace shl
{
    // Declare all the nodes.

    // Correspondence: Single-case grammar rule.

    struct node_program;
    struct node_scope;

    // Correspondence: Multi-case grammar rule.

    struct node_statement;
    struct node_expression;
    struct node_term;
    struct node_binary_operator;

    // Correspondence: Trivial grammar rule.

    struct node_integer_literal;
    struct node_identifier;

    // Correspondence: Non-trivial grammar rule case.

    struct node_return;
    struct node_declare_identifier;
    struct node_if;
    struct node_binary_expression;
    struct node_parenthesised_expression;

    // Correspondence: Binary-only operators.

    struct node_forward_slash {};
    struct node_percent {};

    // Correspondence: Binary and unary operators.

    struct node_asterisk {};
    struct node_plus {};
    struct node_minus {};

    // Correspondence: Unary-only operators.
    // struct node_tilda {};


    // Define all the nodes in the same order.

    struct node_program
    {
        std::vector<node_statement*> _statements;
    };

    struct node_scope
    {
        std::vector<node_statement*> _statements;
    };


    struct node_statement
    {
        std::variant<node_scope*, node_return*, node_declare_identifier*, node_if*> value;
    };

    struct node_expression
    {
        std::variant<node_term*, node_binary_expression*> value;
    };

    struct node_term
    {
        std::variant<node_integer_literal*, node_identifier*, node_parenthesised_expression*> value;
    };

    struct node_binary_operator
    {
        std::variant<node_forward_slash*, node_percent*, node_asterisk*, node_plus*, node_minus*> value;
    };


    struct node_integer_literal
    {
        token _token;
    };

    struct node_identifier
    {
        token _token;
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

    struct node_if
    {
        node_expression* _expression;
        node_scope* _scope;
    };

    struct node_binary_expression
    {
        node_expression* _expression_left;
        node_binary_operator* _binary_operator;
        node_expression* _expression_right;
    };

    struct node_parenthesised_expression
    {
        node_expression* _expression;
    };
} // namespace shl
