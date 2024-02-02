#pragma once

#include "consumer.hpp"
#include "token.hpp"
#include <optional>
#include <string>
#include <vector>

namespace shl
{
    class lexer : consumer<std::string>
    {
    public:
        [[nodiscard]] explicit lexer(const std::string& source_code) noexcept : consumer<std::string>(source_code) {}
        [[nodiscard]] explicit lexer(std::string&& source_code) noexcept : consumer<std::string>(std::move(source_code)) {}

        [[nodiscard]] std::vector<token> operator()();

    private:
        [[nodiscard]] bool try_consume_whitespace_and_comments();
        [[nodiscard]] std::optional<std::string_view> try_consume_integer_literal();
        [[nodiscard]] std::optional<std::string_view> try_consume_identifier();

        using token_pred = bool(*)(char);
        [[nodiscard]] std::optional<std::string_view> try_consume_token(token_pred first_pred, token_pred rest_pred);
        [[nodiscard]] std::optional<std::string_view> try_consume_token(token_pred char_pred);
        [[nodiscard]] std::optional<std::string_view> try_consume_token(std::string_view wanted);
        [[nodiscard]] std::optional<std::string_view> try_consume_token(char wanted);

    private:
        void on_newline();
        std::uint32_t get_column_count();

    private:
        std::uint32_t line_number = 1;
        const_iterator column_iterator;
    };
} // namespace shl
