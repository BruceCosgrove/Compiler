#include "tokenizer.hpp"
#include "error.hpp"
#include <cctype>

namespace shl
{
    std::vector<token> tokenizer::operator()()
    {
        std::vector<token> tokens;

        while (auto c = peek())
        {
            if (try_consume_token_value(c, &std::isspace, &std::isspace))
                continue;
            else if (auto token_value = try_consume_token_value(c, &std::isdigit, &std::isdigit))
                tokens.emplace_back(token_type::integer_literal_, token_value);
            else if (auto token_value = try_consume_token_value(c, &std::isalpha, &std::isalnum))
            {
                     if (token_value == "return") tokens.emplace_back(token_type::return_);
                else if (token_value == "let")    tokens.emplace_back(token_type::let_);
                else if (token_value == "if")     tokens.emplace_back(token_type::if_);
                else tokens.emplace_back(token_type::identifier_, token_value);
            }
            else if (try_consume_token_char(c, '(')) tokens.emplace_back(token_type::open_parenthesis_);
            else if (try_consume_token_char(c, ')')) tokens.emplace_back(token_type::close_parenthesis_);
            else if (try_consume_token_char(c, '{')) tokens.emplace_back(token_type::open_brace_);
            else if (try_consume_token_char(c, '}')) tokens.emplace_back(token_type::close_brace_);
            else if (try_consume_token_char(c, '[')) tokens.emplace_back(token_type::open_bracket_);
            else if (try_consume_token_char(c, ']')) tokens.emplace_back(token_type::close_bracket_);
            else if (try_consume_token_char(c, ';')) tokens.emplace_back(token_type::semicolon_);
            else if (try_consume_token_char(c, '=')) tokens.emplace_back(token_type::equals_);

            else if (try_consume_token_char(c, '/')) tokens.emplace_back(token_type::forward_slash_);
            else if (try_consume_token_char(c, '%')) tokens.emplace_back(token_type::percent_);
            else if (try_consume_token_char(c, '*')) tokens.emplace_back(token_type::asterisk_);
            else if (try_consume_token_char(c, '+')) tokens.emplace_back(token_type::plus_);
            else if (try_consume_token_char(c, '-')) tokens.emplace_back(token_type::minus_);

            else error_exit("Invalid character.");
        }

        reset();
        return tokens;
    }

    std::optional<std::string_view> tokenizer::try_consume_token_value(std::optional<char>& c, token_pred first_pred, token_pred rest_pred)
    {
        if (!first_pred(*c))
            return std::nullopt;

        auto token_begin_it = iterator();
        consume();
        auto token_end_it = iterator();

        if (rest_pred)
        {
            while ((c = peek()) && rest_pred(*c))
            {
                ++token_end_it;
                consume();
            }
        }

        return std::string_view(token_begin_it, token_end_it);
    }

    std::optional<std::string_view> tokenizer::try_consume_token_char(std::optional<char>& c, char wanted)
    {
        if (*c != wanted)
            return std::nullopt;

        auto token_begin_it = iterator();
        consume();
        return std::string_view(token_begin_it, iterator());
    }
} // namespace shl
