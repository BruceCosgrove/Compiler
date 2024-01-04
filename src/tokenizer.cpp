#include "tokenizer.hpp"
#include "error.hpp"
#include <cctype>
#include <iostream> // DEBUG

namespace shl
{
    std::vector<token> tokenizer::tokenize()
    {
        std::vector<token> tokens;

        while (auto c = peek())
        {
            if (try_consume_token_value(c, &std::isspace))
                continue;
            if (auto token_value = try_consume_token_value(c, &std::isalpha, &std::isalnum))
            {
                if (token_value == "return") tokens.emplace_back(token_type::_return);
                else error_exit("Invalid identifier.");
            }
            else if (auto token_value = try_consume_token_value(c, &std::isdigit, &std::isdigit))
                tokens.emplace_back(token_type::_integer_literal, token_value);
            else if (try_consume_token_value(c, [](int c) -> int { return c == ';'; }))
                tokens.emplace_back(token_type::_semicolon);
            else error_exit("Invalid character.");
        }

        reset();
        return tokens;
    }

    std::optional<std::string_view> tokenizer::try_consume_token_value(std::optional<char>& c, const token_pred first_pred, const token_pred rest_pred)
    {
        if (!first_pred(c.value()))
            return std::nullopt;

        const size_type token_begin_index = index();
        consume();
        size_type token_end_index = index();

        if (rest_pred)
        {
            while ((c = peek()) && rest_pred(c.value()))
            {
                ++token_end_index;
                consume();
            }
        }

        return std::string_view(container()).substr(token_begin_index, token_end_index - token_begin_index);
    }
} // namespace shl
