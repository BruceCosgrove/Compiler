#pragma once

#include "consumer.hpp"
#include "token.hpp"
#include "token_nodes.hpp"
#include <vector>

namespace shl
{
    class parser : consumer<std::vector<token>>
    {
    public:
        [[nodiscard]] inline explicit parser(const std::vector<token>& tokens) : consumer<std::vector<token>>(tokens) {}
        [[nodiscard]] inline explicit parser(std::vector<token>&& tokens) noexcept : consumer<std::vector<token>>(std::move(tokens)) {}

        [[nodiscard]] node_program parse();

    private: // Each of these correspond to a node.
        [[nodiscard]] std::optional<node_program> try_parse_program();
        [[nodiscard]] std::optional<node_statement> try_parse_statement();
        [[nodiscard]] std::optional<node_return> try_parse_return();
        [[nodiscard]] std::optional<node_declare_identifier> try_parse_declare_identifier();
        [[nodiscard]] std::optional<node_expression> try_parse_expression();
        [[nodiscard]] std::optional<node_integer_literal> try_parse_integer_literal();
        [[nodiscard]] std::optional<node_identifier> try_parse_identifier();

        [[nodiscard]] std::optional<token> try_consume(token_type type);

    };
} // namespace shl
