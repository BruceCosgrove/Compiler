#include "parser.hpp"
#include <iostream> // DEBUG

namespace shl
{
    node_program* parser::operator()()
    {
        if (auto n_program = try_parse(&parser::try_parse_program, "Invalid program."))
            return n_program;
        return nullptr;
    }

    node_program* parser::try_parse_program()
    {
        auto n_program = _allocator.allocate<node_program>();
        while (peek())
            if (auto n_declaration = try_parse(&parser::try_parse_declaration, "Invalid declaration."))
                n_program->declarations.push_back(n_declaration);
        if (!n_program->declarations.empty())
            return n_program;
        return nullptr;
    }

    node_declaration* parser::try_parse_declaration()
    {
        if (auto n_identifier = try_parse_identifier())
        {
            auto n_declaration = _allocator.allocate<node_declaration>();
            try_consume(token_type::colon_, "Expected ':'.");
            if (try_consume(token_type::let_))
            {
                if (try_consume(token_type::equals_))
                {
                    if (auto n_expression = try_parse(&parser::try_parse_expression, "Expected expression.", 0))
                        n_declaration->n_value = _allocator.allocate<node_definition>(_allocator.allocate<node_define_object>(n_identifier, n_expression));
                }
                else
                    n_declaration->n_value = _allocator.allocate<node_declare_object>(n_identifier);
                try_consume(token_type::semicolon_, "Expected ';'.");
                return n_declaration;
            }
            else if (auto n_function = try_parse_function(true))
            {
                n_declaration->n_value = _allocator.allocate<node_definition>(_allocator.allocate<node_named_function>(n_identifier, n_function));
                return n_declaration;
            }
            else
                error_exit("Ill-formed declaration.");
        }
        return nullptr;
    }

    node_declare_object* parser::try_parse_declare_object()
    {
        if (auto n_identifier = try_parse_identifier())
        {
            try_consume(token_type::colon_, "Expected ':'.");
            try_consume(token_type::let_, "Expected \"let\".");
            return _allocator.allocate<node_declare_object>(n_identifier);
        }
        return nullptr;
    }

    node_function* parser::try_parse_function(bool colon_consumed)
    {
        if (colon_consumed || try_consume(token_type::colon_))
        {
            // An opening parenthesis means there's a function definition.
            if (try_consume(token_type::open_parenthesis_))
            {
                auto n_function = _allocator.allocate<node_function>();
                // Parse any return objects.
                if (auto n_declare_object = try_parse_declare_object())
                {
                    n_function->return_values.push_back(n_declare_object);
                    while (try_consume(token_type::comma_))
                    {
                        n_declare_object = try_parse(&parser::try_parse_declare_object, "Expected return value declaration.");
                        n_function->return_values.push_back(n_declare_object);
                    }
                }
                if (try_consume(token_type::semicolon_))
                {
                    // Parse any parameters.
                    if (auto n_parameter = try_parse_parameter())
                    {
                        n_function->parameters.push_back(n_parameter);
                        while (try_consume(token_type::comma_))
                        {
                            n_parameter = try_parse(&parser::try_parse_parameter, "Expected parameter.");
                            n_function->parameters.push_back(n_parameter);
                        }
                    }
                }
                // Consume the closing parenthesis and equal sign.
                try_consume(token_type::close_parenthesis_, "Expected ')'.");
                try_consume(token_type::equals_, "Expected '='.");
                // Parse the statement.
                n_function->n_statement = try_parse(&parser::try_parse_statement, "Expected statement.");
                return n_function;
            }
        }
        return nullptr;
    }

    node_parameter* parser::try_parse_parameter()
    {
        auto n_parameter_pass = try_parse_parameter_pass();
        if (auto n_identifier = try_parse_identifier())
        {
            if (!n_parameter_pass) // Default parameter passing is in.
                n_parameter_pass = _allocator.allocate<node_parameter_pass>(node_in());
            try_consume(token_type::colon_, "Expected ':'.");
            try_consume(token_type::let_, "Expected \"let\".");
            return _allocator.allocate<node_parameter>(n_parameter_pass, n_identifier);
        }
        else if (n_parameter_pass)
            error_exit("Expected identifier.");
        return nullptr;
    }

    node_scope* parser::try_parse_scope()
    {
        if (try_consume(token_type::open_brace_))
        {
            auto n_scope = _allocator.allocate<node_scope>();
            while (auto n_scoped_statement = try_parse_scoped_statement())
                n_scope->scoped_statements.push_back(n_scoped_statement);
            try_consume(token_type::close_brace_, "Expected '}'.");
            return n_scope;
        }
        return nullptr;
    }

    node_statement* parser::try_parse_statement()
    {
        if (auto n_reassign = try_parse_reassign())
            return _allocator.allocate<node_statement>(n_reassign);
        if (auto n_scope = try_parse_scope())
            return _allocator.allocate<node_statement>(n_scope);
        if (auto n_return = try_parse_return())
            return _allocator.allocate<node_statement>(n_return);
        return nullptr;
    }

    node_scoped_statement* parser::try_parse_scoped_statement()
    {
        if (auto n_statement = try_parse_statement())
            return _allocator.allocate<node_scoped_statement>(n_statement);
        if (auto n_declaration = try_parse_declaration())
            return _allocator.allocate<node_scoped_statement>(n_declaration);
        if (auto n_scoped_if = try_parse_scoped_if())
            return _allocator.allocate<node_scoped_statement>(n_scoped_if);
        return nullptr;
    }

    node_expression* parser::try_parse_expression(std::uint8_t min_precedence)
    {
        auto n_term_lhs = try_parse_term();
        if (!n_term_lhs) return nullptr;

        auto n_expression = _allocator.allocate<node_expression>(n_term_lhs); // create current

        // Operator precedence climbing time!
        while (auto t_op = peek())
        {
            auto precedence = get_operator_precedence(t_op->type);
            if (!precedence || precedence < min_precedence) break;
            auto next_min_precedence = *precedence + 1;

            // TODO: Unary operators.
            // node_unary_expression
            auto n_binary_expression = _allocator.allocate<node_binary_expression>(); // create parent
            n_binary_expression->n_expression_lhs = n_expression; // set left
            n_binary_expression->n_binary_operator = _allocator.allocate<node_binary_operator>(); // set binary operator

            // TODO: Unary operators.
            if (auto n_binary_operator = try_parse_binary_operator())
                n_binary_expression->n_binary_operator = n_binary_operator;

            n_expression = _allocator.allocate<node_expression>(n_binary_expression); // create and continue as parent
            n_binary_expression->n_expression_rhs = try_parse_expression(next_min_precedence); // set parent right
        }

        return n_expression;
    }

    node_term* parser::try_parse_term()
    {
        if (auto n_integer_literal = try_parse_integer_literal())
            return _allocator.allocate<node_term>(n_integer_literal);
        if (auto n_identifier = try_parse_identifier())
            return _allocator.allocate<node_term>(n_identifier);
        if (try_consume(token_type::open_parenthesis_))
        {
            auto n_term = _allocator.allocate<node_term>();
            if (auto n_expression = try_parse(&parser::try_parse_expression, "Expected expression.", 0))
                n_term->n_value = _allocator.allocate<node_parenthesised_expression>(n_expression);
            try_consume(token_type::close_parenthesis_, "Expected ')'.");
            return n_term;
        }
        return nullptr;
    }

    node_binary_operator* parser::try_parse_binary_operator()
    {
        if (try_consume(token_type::forward_slash_)) return _allocator.allocate<node_binary_operator>(node_forward_slash(token_type::forward_slash_));
        if (try_consume(token_type::percent_))       return _allocator.allocate<node_binary_operator>(node_percent(token_type::percent_));

        if (try_consume(token_type::asterisk_))      return _allocator.allocate<node_binary_operator>(node_asterisk(token_type::asterisk_));
        if (try_consume(token_type::plus_))          return _allocator.allocate<node_binary_operator>(node_plus(token_type::plus_));
        if (try_consume(token_type::minus_))         return _allocator.allocate<node_binary_operator>(node_minus(token_type::minus_));

        // if (try_consume(token_type::tilde_))         return _allocator.allocate<node_unary_operator>(node_tilde_(token_type::tilde_));
        return nullptr;
    }

    node_parameter_pass* parser::try_parse_parameter_pass()
    {
        if (try_consume(token_type::in_))    return _allocator.allocate<node_parameter_pass>(node_in());
        if (try_consume(token_type::out_))   return _allocator.allocate<node_parameter_pass>(node_out());
        if (try_consume(token_type::inout_)) return _allocator.allocate<node_parameter_pass>(node_inout());
        if (try_consume(token_type::copy_))  return _allocator.allocate<node_parameter_pass>(node_copy());
        if (try_consume(token_type::move_))  return _allocator.allocate<node_parameter_pass>(node_move());
        return nullptr;
    }

    node_return* parser::try_parse_return()
    {
        if (try_consume(token_type::return_))
        {
            try_consume(token_type::semicolon_, "Expected ';'.");
            return _allocator.allocate<node_return>();
        }
        return nullptr;
    }

    node_reassign* parser::try_parse_reassign()
    {
        if (peek(1) && peek(1)->type == token_type::equals_)
        {
            if (auto n_identifier = try_parse_identifier())
            {
                auto n_reassign = _allocator.allocate<node_reassign>(n_identifier);
                try_consume(token_type::equals_, "Expected '='.");
                if (auto n_expression = try_parse(&parser::try_parse_expression, "Expected expression.", 0))
                    n_reassign->n_expression = n_expression;
                try_consume(token_type::semicolon_, "Expected ';'.");
                return n_reassign;
            }
        }
        return nullptr;
    }

    node_if* parser::try_parse_if(bool if_consumed)
    {
        if (if_consumed || try_consume(token_type::if_))
        {
            auto n_if = _allocator.allocate<node_if>();
            if (auto n_expression = try_parse(&parser::try_parse_expression, "Expected expression.", 0))
                n_if->n_expression = n_expression;
            if (auto n_statement = try_parse(&parser::try_parse_statement, "Expected statement."))
                n_if->n_statement = n_statement;
            return n_if;
        }
        return nullptr;
    }

    node_scoped_if* parser::try_parse_scoped_if()
    {
        if (auto n_if = try_parse_if())
        {
            auto n_scoped_if = _allocator.allocate<node_scoped_if>(std::vector{n_if});
            while (try_consume(token_type::elif_))
                n_scoped_if->ifs.push_back(try_parse(&parser::try_parse_if, "Expected expression and statement.", true));
            if (try_consume(token_type::else_))
                n_scoped_if->ifs.push_back(_allocator.allocate<node_if>(nullptr, try_parse(&parser::try_parse_statement, "Expected statement.")));
            return n_scoped_if;
        }
        return nullptr;
    }

    node_integer_literal* parser::try_parse_integer_literal()
    {
        if (auto t_integer_literal = try_consume(token_type::integer_literal_))
            return _allocator.allocate<node_integer_literal>(*t_integer_literal->value);
        return nullptr;
    }

    node_identifier* parser::try_parse_identifier()
    {
        if (auto t_identifier = try_consume(token_type::identifier_))
            return _allocator.allocate<node_identifier>(*t_identifier->value);
        return nullptr;
    }

    std::optional<token> parser::try_consume(token_type type)
    {
        if (auto t = peek(); t && t->type == type)
            return *consume();
        return std::nullopt;
    }

    token parser::try_consume(token_type type, std::string_view error_message)
    {
        if (auto t = peek(); t && t->type == type)
            return *consume();
        error_exit(error_message);
    }
} // namespace shl
