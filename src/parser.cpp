#include "parser.hpp"
#include "error.hpp"

namespace shl
{
    node_program parser::parse()
    {
        if (auto n = try_parse_program())
            return *n;
        else
            error_exit("Invalid program.");
        return {};
    }

    std::optional<node_program> parser::try_parse_program()
    {
        node_program _program;
        while (peek())
        {
            if (auto n = try_parse_statement())
                _program._statements.push_back(*n);
            else
                error_exit("Invalid statement.");
        }
        if (!_program._statements.empty())
            return _program;
        return std::nullopt;
    }

    std::optional<node_statement> parser::try_parse_statement()
    {
        if (auto n = try_parse_return())
            return *n;
        if (auto n = try_parse_declare_identifier())
            return *n;
        return std::nullopt;
    }

    std::optional<node_return> parser::try_parse_return()
    {
        if (try_consume(token_type::_return))
        {
            node_return _return;
            if (auto n = try_parse_expression())
                _return._expression = *n;
            else
                error_exit("Expected expression.");
            if (!try_consume(token_type::_semicolon))
                error_exit("Expected ';'.");
            return _return;
        }
        return std::nullopt;
    }

    std::optional<node_declare_identifier> parser::try_parse_declare_identifier()
    {
        if (try_consume(token_type::_let))
        {
            node_declare_identifier _declare_identifier;
            if (auto n = try_parse_identifier())
                _declare_identifier._identifier = *n;
            else
                error_exit("Expected identifier.");
            if (!try_consume(token_type::_equals))
                error_exit("Expected '='.");
            if (auto n = try_parse_expression())
                _declare_identifier._expression = *n;
            else
                error_exit("Expected expression.");
            if (!try_consume(token_type::_semicolon))
                error_exit("Expected ';'.");
            return _declare_identifier;
        }
        return std::nullopt;
    }

    std::optional<node_expression> parser::try_parse_expression()
    {
        if (auto n = try_parse_integer_literal())
            return *n;
        if (auto n = try_parse_identifier())
            return *n;
        return std::nullopt;
    }

    std::optional<node_integer_literal> parser::try_parse_integer_literal()
    {
        if (auto t = try_consume(token_type::_integer_literal))
            return node_integer_literal(*t);
        return std::nullopt;
    }

    std::optional<node_identifier> parser::try_parse_identifier()
    {
        if (auto t = try_consume(token_type::_identifier))
            return node_identifier(*t);
        return std::nullopt;
    }

    std::optional<token> parser::try_consume(token_type type)
    {
        if (auto t = peek(); t && t->type == type)
            return consume();
        return std::nullopt;
    }
} // namespace shl
