#include "parser.hpp"
#include "error.hpp"

namespace shl
{
    node_program* parser::operator()()
    {
        if (auto n = try_parse_program())
            return n;
        else
            error_exit("Invalid program.");
        return nullptr;
    }

    node_program* parser::try_parse_program()
    {
        auto _program = _allocator.allocate<node_program>();
        while (peek())
        {
            if (auto n = try_parse_statement())
                _program->_statements.push_back(n);
            else
                error_exit("Invalid statement.");
        }
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

    node_return* parser::try_parse_return()
    {
        if (try_consume(token_type::_return))
        {
            auto _return = _allocator.allocate<node_return>();
            if (auto n = try_parse_expression())
                _return->_expression = n;
            else
                error_exit("Expected expression.");
            if (!try_consume(token_type::_semicolon))
                error_exit("Expected ';'.");
            return _return;
        }
        return nullptr;
    }

    node_declare_identifier* parser::try_parse_declare_identifier()
    {
        if (try_consume(token_type::_let))
        {
            auto _declare_identifier = _allocator.allocate<node_declare_identifier>();
            if (auto n = try_parse_identifier())
                _declare_identifier->_identifier = n;
            else
                error_exit("Expected identifier.");
            if (!try_consume(token_type::_equals))
                error_exit("Expected '='.");
            if (auto n = try_parse_expression())
                _declare_identifier->_expression = n;
            else
                error_exit("Expected expression.");
            if (!try_consume(token_type::_semicolon))
                error_exit("Expected ';'.");
            return _declare_identifier;
        }
        return nullptr;
    }

    node_expression* parser::try_parse_expression()
    {
        if (auto n = try_parse_integer_literal())
            return _allocator.allocate<node_expression>(n);
        if (auto n = try_parse_identifier())
            return _allocator.allocate<node_expression>(n);
        // if (auto n = try_parse_binary_expression())
        //     return _allocator.allocate<node_binary_expression>(n);
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

    std::optional<token> parser::try_consume(token_type type)
    {
        if (auto t = peek(); t && t->type == type)
            return consume();
        return std::nullopt;
    }
} // namespace shl
