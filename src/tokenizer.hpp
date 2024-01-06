#pragma once

#include "consumer.hpp"
#include "token.hpp"
#include <optional>
#include <string>
#include <vector>

namespace shl
{
    class tokenizer : consumer<std::string>
    {
    public:
        [[nodiscard]] inline explicit tokenizer(const std::string& source_code) noexcept : consumer<std::string>(source_code) {}
        [[nodiscard]] inline explicit tokenizer(std::string&& source_code) noexcept : consumer<std::string>(std::move(source_code)) {}

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
    };
} // namespace shl
