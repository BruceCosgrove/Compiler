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
        [[nodiscard]] node_expression* try_parse_expression();
        [[nodiscard]] node_term* try_parse_term();

        // Embedded in try_parse_expression to make it parse correctly.
        // [[nodiscard]] node_binary_expression* try_parse_binary_expression();

        [[nodiscard]] node_integer_literal* try_parse_integer_literal();
        [[nodiscard]] node_identifier* try_parse_identifier();
        [[nodiscard]] node_binary_operator* try_parse_binary_operator();

        [[nodiscard]] node_return* try_parse_return();
        [[nodiscard]] node_declare_identifier* try_parse_declare_identifier();

    private:
        [[nodiscard]] std::optional<token> try_consume(const token_type type);
        [[nodiscard]] token try_consume(const token_type type, const std::string_view error_message);

    private:
        arena_allocator _allocator{1024 * 1024}; // 1 MiB blocks.
    };
} // namespace shl
