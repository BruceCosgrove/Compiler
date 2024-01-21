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
        [[nodiscard]] explicit parser(const std::vector<token>& tokens) : consumer<std::vector<token>>(tokens) {}
        [[nodiscard]] explicit parser(std::vector<token>&& tokens) noexcept : consumer<std::vector<token>>(std::move(tokens)) {}

        [[nodiscard]] node_program* operator()();

    private:
        // Correspondence: Cased grammar rule.

        [[nodiscard]] node_program* try_parse_program();
        [[nodiscard]] node_declaration* try_parse_declaration();
        [[nodiscard]] node_declare_object* try_parse_declare_object(node_identifier* n_name, bool optional_type);
        [[nodiscard]] node_function* try_parse_function(bool colon_consumed = false);
        [[nodiscard]] node_parameter* try_parse_parameter();
        [[nodiscard]] node_scope* try_parse_scope();
        [[nodiscard]] node_statement* try_parse_statement();
        [[nodiscard]] node_scoped_statement* try_parse_scoped_statement();
        // Also parses operator precedence.
        [[nodiscard]] node_expression* try_parse_expression(std::uint8_t min_precedence);
        [[nodiscard]] node_term* try_parse_term();

        // Correspondence: Non-trivial grammar rule case.

        [[nodiscard]] node_return* try_parse_return();
        [[nodiscard]] node_reassign* try_parse_reassign();
        [[nodiscard]] node_if* try_parse_if(bool if_consumed = false);
        [[nodiscard]] node_scoped_if* try_parse_scoped_if();
        // [[nodiscard]] node_binary_expression* try_parse_binary_expression();

        // Correspondence: Trivial grammar rule.

        [[nodiscard]] node_binary_operator* try_parse_binary_operator();
        [[nodiscard]] node_parameter_pass* try_parse_parameter_pass();
        [[nodiscard]] node_integer_literal* try_parse_integer_literal();
        [[nodiscard]] node_identifier* try_parse_identifier();

    private:
        [[nodiscard]] std::optional<token> try_consume(token_type type);
        token try_consume(token_type type, std::string_view error_message);

        template <typename Func, typename... Args>
        [[nodiscard]] auto try_parse(Func func, std::string_view error_message, Args&&... args)
        {
            if (auto n = (this->*func)(std::forward<Args>(args)...)) return n;
            else error(std::nullopt, error_message);
        }

    private:
        [[noreturn]] void error(const std::optional<token>& token, std::string_view error_message);

    private:
        arena_allocator _allocator{1024 * 1024}; // 1 MiB blocks.
    };
} // namespace shl
