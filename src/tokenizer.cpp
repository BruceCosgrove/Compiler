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
            if (auto token_value = try_consume_token_value(c, &std::isalpha, &std::isalnum))
            {
                     if (token_value == "return") tokens.emplace_back(token_type::_return);
                else if (token_value == "let")    tokens.emplace_back(token_type::_let);
                else if (token_value == "if")     tokens.emplace_back(token_type::_if);
                else tokens.emplace_back(token_type::_identifier, token_value);
            }
            else if (auto token_value = try_consume_token_value(c, &std::isdigit, &std::isdigit))
                tokens.emplace_back(token_type::_integer_literal, token_value);
            else if (try_consume_token_char(c, ';')) tokens.emplace_back(token_type::_semicolon);
            else if (try_consume_token_char(c, '(')) tokens.emplace_back(token_type::_open_parenthesis);
            else if (try_consume_token_char(c, ')')) tokens.emplace_back(token_type::_close_parenthesis);
            else if (try_consume_token_char(c, '{')) tokens.emplace_back(token_type::_open_brace);
            else if (try_consume_token_char(c, '}')) tokens.emplace_back(token_type::_close_brace);
            else if (try_consume_token_char(c, '[')) tokens.emplace_back(token_type::_open_bracket);
            else if (try_consume_token_char(c, ']')) tokens.emplace_back(token_type::_close_bracket);
            else if (try_consume_token_char(c, '=')) tokens.emplace_back(token_type::_equals);
            else if (try_consume_token_char(c, '/')) tokens.emplace_back(token_type::_forward_slash);
            else if (try_consume_token_char(c, '%')) tokens.emplace_back(token_type::_percent);
            else if (try_consume_token_char(c, '*')) tokens.emplace_back(token_type::_asterisk);
            else if (try_consume_token_char(c, '+')) tokens.emplace_back(token_type::_plus);
            else if (try_consume_token_char(c, '-')) tokens.emplace_back(token_type::_minus);
            else error_exit("Invalid character.");
        }

        reset();
        return tokens;
    }

    std::optional<std::string_view> tokenizer::try_consume_token_value(std::optional<char>& c, const token_pred first_pred, const token_pred rest_pred)
    {
        if (!first_pred(*c))
            return std::nullopt;

        const auto token_begin_it = iterator();
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

    std::optional<std::string_view> tokenizer::try_consume_token_char(std::optional<char>& c, const char wanted)
    {
        if (*c != wanted)
            return std::nullopt;

        const auto token_begin_it = iterator();
        consume();
        return std::string_view(token_begin_it, iterator());
    }
} // namespace shl
