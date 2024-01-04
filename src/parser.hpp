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

        [[nodiscard]] node_return parse();

    private:
        [[nodiscard]] std::optional<node_return> parse_return();
        [[nodiscard]] std::optional<node_expression> parse_expression();

        [[nodiscard]] std::optional<token> try_consume(token_type type);

    };
} // namespace shl
