#include "parser.hpp"

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

    node_statement* parser::try_parse_statement()
    {
        if (auto n = try_parse_return())
            return _allocator.allocate<node_statement>(n);
        if (auto n = try_parse_declare_identifier())
            return _allocator.allocate<node_statement>(n);
        return nullptr;
    }

    node_expression* parser::try_parse_expression()
    {
        if (auto n1 = try_parse_term())
        {
            auto term_expression = _allocator.allocate<node_expression>(n1);
            if (auto n2 = try_parse_binary_operator())
            {
                if (auto n3 = try_parse(&parser::try_parse_expression, "TODO: Unary operators."))
                    return _allocator.allocate<node_expression>(_allocator.allocate<node_binary_expression>(term_expression, n2, n3));
            }
            else
                return term_expression;
        }
        return nullptr;
    }

    node_term* parser::try_parse_term()
    {
        if (auto n = try_parse_integer_literal())
            return _allocator.allocate<node_term>(n);
        if (auto n = try_parse_identifier())
            return _allocator.allocate<node_term>(n);
        return nullptr;
    }

    // Embedded in try_parse_expression to make it parse correctly.
    // [[nodiscard]] node_binary_expression* parser::try_parse_binary_expression() { static_assert(false); return nullptr; }

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

    node_binary_operator* parser::try_parse_binary_operator()
    {
        // if (auto t = try_consume(token_type::_asterisk))
        //     return _allocator.allocate<node_binary_operator>(_allocator.allocate<node_asterisk>());
        // if (auto t = try_consume(token_type::_forward_slash))
        //     return _allocator.allocate<node_binary_operator>(_allocator.allocate<node_forward_slash>());
        // if (auto t = try_consume(token_type::_percent))
        //     return _allocator.allocate<node_binary_operator>(_allocator.allocate<node_percent>());
        if (auto t = try_consume(token_type::_plus))
            return _allocator.allocate<node_binary_operator>(_allocator.allocate<node_plus>());
        // if (auto t = try_consume(token_type::_minus))
        //     return _allocator.allocate<node_binary_operator>(_allocator.allocate<node_minus>());
        return nullptr;
    }

    node_return* parser::try_parse_return()
    {
        if (try_consume(token_type::_return))
        {
            auto _return = _allocator.allocate<node_return>();
            if (auto n = try_parse(&parser::try_parse_expression, "Expected expression."))
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
            if (auto n = try_parse(&parser::try_parse_expression, "Expected expression."))
                _declare_identifier->_expression = n;
            try_consume(token_type::_semicolon, "Expected ';'.");
            return _declare_identifier;
        }
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
