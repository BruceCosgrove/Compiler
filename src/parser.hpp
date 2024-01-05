#pragma once

#include "arena_allocator.hpp"
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

        [[nodiscard]] node_program* operator()();

    private: // Each of these correspond to a node.
        [[nodiscard]] node_program* try_parse_program();
        [[nodiscard]] node_statement* try_parse_statement();
        [[nodiscard]] node_return* try_parse_return();
        [[nodiscard]] node_declare_identifier* try_parse_declare_identifier();
        [[nodiscard]] node_expression* try_parse_expression();
        [[nodiscard]] node_integer_literal* try_parse_integer_literal();
        [[nodiscard]] node_identifier* try_parse_identifier();

        [[nodiscard]] std::optional<token> try_consume(token_type type);

    private:
        arena_allocator _allocator{1024 * 1024}; // 1 MiB blocks.
    };
} // namespace shl
