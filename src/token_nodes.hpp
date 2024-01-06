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
        std::vector<node_statement*> statements;
    };

    struct node_scope
    {
        std::vector<node_statement*> statements;
    };


    struct node_statement
    {
        std::variant<node_scope*, node_return*, node_declare_identifier*, node_if*> n_value;
    };

    struct node_expression
    {
        std::variant<node_term*, node_binary_expression*> n_value;
    };

    struct node_term
    {
        std::variant<node_integer_literal*, node_identifier*, node_parenthesised_expression*> n_value;
    };

    struct node_binary_operator
    {
        std::variant<node_forward_slash*, node_percent*, node_asterisk*, node_plus*, node_minus*> n_value;
    };


    struct node_integer_literal
    {
        std::string_view value;
    };

    struct node_identifier
    {
        std::string_view value;
    };


    struct node_return
    {
        node_expression* n_expression;
    };

    struct node_declare_identifier
    {
        node_identifier* n_identifier;
        node_expression* n_expression;
    };

    struct node_if
    {
        node_expression* n_expression;
        node_scope* n_scope;
    };

    struct node_binary_expression
    {
        node_expression* n_expression_lhs;
        node_binary_operator* n_binary_operator;
        node_expression* n_expression_rhs;
    };

    struct node_parenthesised_expression
    {
        node_expression* n_expression;
    };
} // namespace shl
