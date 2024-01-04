#pragma once

#include "tokens.hpp"
#include "consumer.hpp"
#include <optional>
#include <string>
#include <vector>

namespace shl
{
    class tokenizer : consumer<std::string>
    {
    public:
        [[nodiscard]] inline tokenizer(const std::string& source_code) noexcept : consumer<std::string>(source_code) {}
        [[nodiscard]] inline tokenizer(std::string&& source_code) noexcept : consumer<std::string>(std::move(source_code)) {}

        [[nodiscard]] std::vector<token> tokenize();

    private:
        using token_pred = int(*)(int);
        [[nodiscard]] std::optional<std::string_view> try_consume_token_value(std::optional<char>& c, const token_pred first_pred, const token_pred rest_pred = nullptr);
    };
} // namespace shl
