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

        [[nodiscard]] std::vector<token> tokenize();

    private:
        using token_pred = int(*)(int);
        [[nodiscard]] std::optional<std::string_view> try_consume_token_value(std::optional<char>& c, const token_pred first_pred, const token_pred rest_pred);
        [[nodiscard]] std::optional<std::string_view> try_consume_token_char(std::optional<char>& c, const char wanted);
    };
} // namespace shl
