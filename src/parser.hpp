#pragma once

#include "arena_allocator.hpp"
#include "consumer.hpp"
#include "error.hpp"
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

    private:
        // Correspondence: Single-case grammar rule.

        [[nodiscard]] node_program* try_parse_program();
        [[nodiscard]] node_scope* try_parse_scope();

        // Correspondence: Multi-case grammar rule.

        [[nodiscard]] node_statement* try_parse_statement();
        // Also parses operator precedence.
        [[nodiscard]] node_expression* try_parse_expression(const std::uint8_t min_precedence = 0);
        [[nodiscard]] node_term* try_parse_term();
        [[nodiscard]] node_binary_operator* try_parse_binary_operator();

        // Correspondence: Non-trivial grammar rule case.

        [[nodiscard]] node_return* try_parse_return();
        [[nodiscard]] node_declare_identifier* try_parse_declare_identifier();
        [[nodiscard]] node_if* try_parse_if();

        // Correspondence: Trivial grammar rule.

        [[nodiscard]] node_integer_literal* try_parse_integer_literal();
        [[nodiscard]] node_identifier* try_parse_identifier();

    private:
        [[nodiscard]] std::optional<token> try_consume(const token_type type);
        token try_consume(const token_type type, const std::string_view error_message);

        template <typename Func, typename... Args>
        [[nodiscard]] auto try_parse(Func func, const std::string_view error_message, Args&&... args)
        {
            if (auto n = (this->*func)(std::forward<Args>(args)...)) return n;
            else error_exit("Invalid program.");
        }

    private:
        arena_allocator _allocator{1024 * 1024}; // 1 MiB blocks.
    };
} // namespace shl
