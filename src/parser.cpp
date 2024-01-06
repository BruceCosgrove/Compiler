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
            if (auto n_statement = try_parse(&parser::try_parse_statement, "Invalid statement."))
                n_program->statements.push_back(n_statement);
        if (!n_program->statements.empty())
            return n_program;
        return nullptr;
    }

    node_scope* parser::try_parse_scope()
    {
        if (try_consume(token_type::open_brace_))
        {
            auto n_scope = _allocator.allocate<node_scope>();
            while (auto n_statement = try_parse_statement())
                n_scope->statements.push_back(n_statement);
            try_consume(token_type::close_brace_, "Expected '}'.");
            return n_scope;
        }
        return nullptr;
    }

    node_statement* parser::try_parse_statement()
    {
        if (auto n_scope = try_parse_scope())
            return _allocator.allocate<node_statement>(n_scope);
        if (auto n_return = try_parse_return())
            return _allocator.allocate<node_statement>(n_return);
        if (auto n_declare_identifier = try_parse_declare_identifier())
            return _allocator.allocate<node_statement>(n_declare_identifier);
        if (auto n_if = try_parse_if())
            return _allocator.allocate<node_statement>(n_if);
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

            auto n_binary_expression = _allocator.allocate<node_binary_expression>(); // create parent
            n_binary_expression->n_expression_lhs = n_expression; // set left
            n_binary_expression->n_binary_operator = _allocator.allocate<node_binary_operator>(); // set operator

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
        if (try_consume(token_type::forward_slash_)) return _allocator.allocate<node_binary_operator>(_allocator.allocate<node_forward_slash>());
        if (try_consume(token_type::percent_))       return _allocator.allocate<node_binary_operator>(_allocator.allocate<node_percent>());

        if (try_consume(token_type::asterisk_))      return _allocator.allocate<node_binary_operator>(_allocator.allocate<node_asterisk>());
        if (try_consume(token_type::plus_))          return _allocator.allocate<node_binary_operator>(_allocator.allocate<node_plus>());
        if (try_consume(token_type::minus_))         return _allocator.allocate<node_binary_operator>(_allocator.allocate<node_minus>());

        return nullptr;
    }

    node_return* parser::try_parse_return()
    {
        if (try_consume(token_type::return_))
        {
            auto n_return = _allocator.allocate<node_return>();
            if (auto n_expression = try_parse(&parser::try_parse_expression, "Expected expression.", 0))
                n_return->n_expression = n_expression;
            try_consume(token_type::semicolon_, "Expected ';'.");
            return n_return;
        }
        return nullptr;
    }

    node_declare_identifier* parser::try_parse_declare_identifier()
    {
        if (try_consume(token_type::let_))
        {
            auto n_declare_identifier = _allocator.allocate<node_declare_identifier>();
            if (auto n_identifier = try_parse(&parser::try_parse_identifier, "Expected identifier."))
                n_declare_identifier->n_identifier = n_identifier;
            try_consume(token_type::equals_, "Expected '='.");
            if (auto n_expression = try_parse(&parser::try_parse_expression, "Expected expression.", 0))
                n_declare_identifier->n_expression = n_expression;
            try_consume(token_type::semicolon_, "Expected ';'.");
            return n_declare_identifier;
        }
        return nullptr;
    }

    node_if* parser::try_parse_if()
    {
        if (try_consume(token_type::if_))
        {
            auto n_if = _allocator.allocate<node_if>();
            if (auto n_expression = try_parse(&parser::try_parse_expression, "Expected expression.", 0))
                n_if->n_expression = n_expression;
            if (auto n_scope = try_parse(&parser::try_parse_scope, "Expected scope."))
                n_if->n_scope = n_scope;
            return n_if;
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
