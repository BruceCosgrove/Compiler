#pragma once

#include "tokens.hpp"
#include "consumer.hpp"
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
    };
} // namespace shl
