#include "parser.hpp"
#include "error.hpp"

namespace shl
{
    node_return parser::parse()
    {
        if (auto n = parse_return()) return n.value();
        else error_exit("Invalid program.");
        return {};
    }

    std::optional<node_return> parser::parse_return()
    {
        std::optional<node_return> _return;

        if (try_consume(token_type::_return))
        {
            if (auto n = parse_expression())
                _return = node_return(n.value());
            else error_exit("Expected expression.");
            if (!try_consume(token_type::_semicolon))
                error_exit("Expected ';'.");
        }

        return _return;
    }

    std::optional<node_expression> parser::parse_expression()
    {
        if (auto t = try_consume(token_type::_integer_literal))
            return node_expression(t.value());
        return std::nullopt;
    }

    std::optional<token> parser::try_consume(token_type type)
    {
        if (auto t = peek(); t && t.value().type == type)
            return consume();
        return std::nullopt;
    }
} // namespace shl
