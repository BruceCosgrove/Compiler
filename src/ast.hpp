#pragma once

#include "token.hpp"
#include <variant>
#include <vector>

namespace shl
{
    // Declare all the nodes.

    // Correspondence: Cased grammar rule.

    struct node_program;
    struct node_declaration;
    struct node_definition;
    struct node_declare_object;
    struct node_define_object;
    struct node_function;
    struct node_named_function;
    struct node_parameter;
    struct node_scope;
    struct node_statement;
    struct node_scoped_statement;
    struct node_expression;
    struct node_term;

    // Correspondence: Non-trivial grammar rule case.

    struct node_return;
    struct node_if;
    struct node_reassign;
    struct node_scoped_if;
    struct node_binary_expression;

    // Correspondence: Trivial grammar rule.

    struct node_binary_operator;
    struct node_parameter_pass;
    struct node_integer_literal;
    struct node_identifier;

    // Correspondence: Binary-only operators.

    struct node_forward_slash { token_type type; };
    struct node_percent { token_type type; };

    // Correspondence: Binary and unary operators.

    struct node_asterisk { token_type type; };
    struct node_plus { token_type type; };
    struct node_minus { token_type type; };

    // Correspondence: Unary-only operators.
    // struct node_tilda {};

    // Correspondence: Parameter passing.

    struct node_in {};
    struct node_out {};
    struct node_inout {};
    struct node_copy {};
    struct node_move {};


    // All node types.
    #define NODE_TYPES \
        std::monostate, \
        node_program*, \
        node_declaration*, \
        node_definition*, \
        node_declare_object*, \
        node_define_object*, \
        node_function*, \
        node_named_function*, \
        node_parameter*, \
        node_scope*, \
        node_statement*, \
        node_scoped_statement*, \
        node_expression*, \
        node_term*, \
        node_return*, \
        node_if*, \
        node_reassign*, \
        node_scoped_if*, \
        node_binary_expression*, \
        node_binary_operator*, \
        node_parameter_pass*, \
        node_integer_literal*, \
        node_identifier*, \
        node_forward_slash, \
        node_percent, \
        node_asterisk, \
        node_plus, \
        node_minus, \
        node_in, \
        node_out, \
        node_inout, \
        node_copy, \
        node_move

    // Variant of all node types.
    using nodes = std::variant<NODE_TYPES>;


    // Define all the undefined nodes in the same order.

    struct node_program
    {
        std::vector<node_declaration*> declarations; // 1 or more.
    };

    struct node_declaration
    {
        std::variant<node_definition*, node_declare_object*> n_value;
    };

    struct node_definition
    {
        std::variant<node_define_object*, node_named_function*> n_value;
    };

    struct node_declare_object
    {
        node_identifier* n_name;
        node_identifier* n_type;
    };

    struct node_define_object
    {
        node_identifier* n_name;
        node_identifier* n_type;
        node_expression* n_expression;
    };

    struct node_function
    {
        std::vector<node_declare_object*> return_values; // 0 or more
        std::vector<node_parameter*> parameters; // 0 or more
        node_statement* n_statement;
    };

    struct node_named_function
    {
        node_identifier* n_name;
        node_function* n_function;
    };

    struct node_parameter
    {
        node_parameter_pass* n_pass;
        node_declare_object* n_declare_object;
    };

    struct node_scope
    {
        std::vector<node_scoped_statement*> scoped_statements; // 0 or more
    };

    struct node_statement
    {
        std::variant<std::monostate, node_scope*, node_if*, node_return*, node_reassign*> n_value;
    };

    struct node_scoped_statement
    {
        std::variant<node_statement*, node_declaration*, node_scoped_if*> n_value;
    };

    struct node_expression
    {
        std::variant<node_term*, node_binary_expression*> n_value;
    };

    struct node_term
    {
        std::variant<node_integer_literal*, node_identifier*, node_expression*> n_value;
    };


    struct node_return
    {
        // Returning an expression is invalid since the return values
        // are already named and allocated in the caller's stack frame.
        // However, this does not stop one from returning from a function
        // early, so long as all out objects are initialized.
    };

    struct node_if
    {
        node_expression* n_expression;
        node_statement* n_statement;
    };

    struct node_reassign
    {
        node_identifier* n_identifier;
        node_expression* n_expression;
    };

    struct node_scoped_if
    {
        std::vector<node_if*> ifs;
    };

    struct node_binary_expression
    {
        node_expression* n_expression_lhs;
        node_binary_operator* n_binary_operator;
        node_expression* n_expression_rhs;
    };


    struct node_binary_operator
    {
        std::variant<node_forward_slash, node_percent, node_asterisk, node_plus, node_minus> n_value;
    };

    struct node_parameter_pass
    {
        std::variant<node_in, node_out, node_inout, node_copy, node_move> n_value;
    };

    struct node_integer_literal
    {
        std::string_view value;
    };

    struct node_identifier
    {
        std::string_view value;
    };
} // namespace shl
