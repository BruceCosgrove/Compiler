#include "parser.hpp"
#include <iostream> // DEBUG

namespace shl
{
    node_program* parser::operator()()
    {
        if (auto n = try_parse(&parser::try_parse_program, "Invalid program."))
            return n;
        return nullptr;
    }

    node_program* parser::try_parse_program()
    {
        auto _program = _allocator.allocate<node_program>();
        while (peek())
            if (auto n = try_parse(&parser::try_parse_statement, "Invalid statement."))
                _program->_statements.push_back(n);
        if (!_program->_statements.empty())
            return _program;
        return nullptr;
    }

    node_scope* parser::try_parse_scope()
    {
        if (try_consume(token_type::_open_brace))
        {
            auto n_scope = _allocator.allocate<node_scope>();
            while (auto n_statement = try_parse_statement())
                n_scope->_statements.push_back(n_statement);
            try_consume(token_type::_close_brace, "Expected '}'.");
            return n_scope;
        }
        return nullptr;
    }

    node_statement* parser::try_parse_statement()
    {
        if (auto n = try_parse_scope())
            return _allocator.allocate<node_statement>(n);
        if (auto n = try_parse_return())
            return _allocator.allocate<node_statement>(n);
        if (auto n = try_parse_declare_identifier())
            return _allocator.allocate<node_statement>(n);
        if (auto n = try_parse_if())
            return _allocator.allocate<node_statement>(n);
        return nullptr;
    }

    node_expression* parser::try_parse_expression(const std::uint8_t min_precedence)
    {
        auto term_left = try_parse_term();
        if (!term_left) return nullptr;

        auto expression = _allocator.allocate<node_expression>(term_left); // create current

        // Operator precedence climbing time!
        while (const auto t_op = peek())
        {
            const auto precedence = get_operator_precedence(t_op->type);
            if (!precedence || precedence < min_precedence) break;
            const auto next_min_precedence = *precedence + 1;

            auto nbe = _allocator.allocate<node_binary_expression>(); // create parent
            nbe->_expression_left = expression; // set left
            nbe->_binary_operator = _allocator.allocate<node_binary_operator>(); // set operator

            if (auto n_binary_operator = try_parse_binary_operator())
                nbe->_binary_operator = n_binary_operator;

            expression = _allocator.allocate<node_expression>(nbe); // create and continue as parent
            nbe->_expression_right = try_parse_expression(next_min_precedence); // set parent right
        }

        return expression;
    }

    node_term* parser::try_parse_term()
    {
        if (auto n = try_parse_integer_literal())
            return _allocator.allocate<node_term>(n);
        if (auto n = try_parse_identifier())
            return _allocator.allocate<node_term>(n);
        if (try_consume(token_type::_open_parenthesis))
        {
            auto _term = _allocator.allocate<node_term>();
            if (auto n = try_parse(&parser::try_parse_expression, "Expected expression.", 0))
                _term->value = _allocator.allocate<node_parenthesised_expression>(n);
            try_consume(token_type::_close_parenthesis, "Expected ')'.");
            return _term;
        }
        return nullptr;
    }

    node_binary_operator* parser::try_parse_binary_operator()
    {
        if (try_consume(token_type::_forward_slash))
            return _allocator.allocate<node_binary_operator>(_allocator.allocate<node_forward_slash>());
        if (try_consume(token_type::_percent))
            return _allocator.allocate<node_binary_operator>(_allocator.allocate<node_percent>());
        if (try_consume(token_type::_asterisk))
            return _allocator.allocate<node_binary_operator>(_allocator.allocate<node_asterisk>());
        if (try_consume(token_type::_plus))
            return _allocator.allocate<node_binary_operator>(_allocator.allocate<node_plus>());
        if (try_consume(token_type::_minus))
            return _allocator.allocate<node_binary_operator>(_allocator.allocate<node_minus>());
        return nullptr;
    }

    node_return* parser::try_parse_return()
    {
        if (try_consume(token_type::_return))
        {
            auto _return = _allocator.allocate<node_return>();
            if (auto n = try_parse(&parser::try_parse_expression, "Expected expression.", 0))
                _return->_expression = n;
            try_consume(token_type::_semicolon, "Expected ';'.");
            return _return;
        }
        return nullptr;
    }

    node_declare_identifier* parser::try_parse_declare_identifier()
    {
        if (try_consume(token_type::_let))
        {
            auto _declare_identifier = _allocator.allocate<node_declare_identifier>();
            if (auto n = try_parse(&parser::try_parse_identifier, "Expected identifier."))
                _declare_identifier->_identifier = n;
            try_consume(token_type::_equals, "Expected '='.");
            if (auto n = try_parse(&parser::try_parse_expression, "Expected expression.", 0))
                _declare_identifier->_expression = n;
            try_consume(token_type::_semicolon, "Expected ';'.");
            return _declare_identifier;
        }
        return nullptr;
    }

    node_if* parser::try_parse_if()
    {
        if (try_consume(token_type::_if))
        {
            auto n_if = _allocator.allocate<node_if>();
            if (auto n_expression = try_parse(&parser::try_parse_expression, "Expected expression.", 0))
                n_if->_expression = n_expression;
            if (auto n_scope = try_parse(&parser::try_parse_scope, "Expected scope."))
                n_if->_scope = n_scope;
            return n_if;
        }
        return nullptr;
    }

    node_integer_literal* parser::try_parse_integer_literal()
    {
        if (auto t = try_consume(token_type::_integer_literal))
            return _allocator.allocate<node_integer_literal>(*t);
        return nullptr;
    }

    node_identifier* parser::try_parse_identifier()
    {
        if (auto t = try_consume(token_type::_identifier))
            return _allocator.allocate<node_identifier>(*t);
        return nullptr;
    }

    std::optional<token> parser::try_consume(const token_type type)
    {
        if (auto t = peek(); t && t->type == type)
            return consume();
        return std::nullopt;
    }

    token parser::try_consume(const token_type type, const std::string_view error_message)
    {
        if (auto t = peek(); t && t->type == type)
            return consume();
        error_exit(error_message);
    }
} // namespace shl
