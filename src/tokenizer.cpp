#include "tokenizer.hpp"
#include "error.hpp"
#include <cctype>

namespace shl
{
    inline bool is_space           (char c) noexcept { return std::isspace(c);             }
    inline bool is_digit_10        (char c) noexcept { return std::isdigit(c);             }
    inline bool is_identifier_first(char c) noexcept { return std::isalpha(c) || c == '_'; }
    inline bool is_identifier_rest (char c) noexcept { return std::isalnum(c) || c == '_'; }
    inline bool is_line_ending     (char c) noexcept { return c == '\n' || c == '\r';      }
    inline bool is_not_line_ending (char c) noexcept { return !is_line_ending(c);          }

    std::vector<token> tokenizer::operator()()
    {
        std::vector<token> tokens;

        while (peek())
        {
            if (try_consume_whitespace_and_comments()) continue;
            else if (auto token_value = try_consume_integer_literal())
                tokens.emplace_back(token_type::integer_literal_, token_value);
            else if (auto token_value = try_consume_identifier())
            {
                     if (token_value == "return") tokens.emplace_back(token_type::return_);
                else if (token_value == "let")    tokens.emplace_back(token_type::let_);
                else if (token_value == "if")     tokens.emplace_back(token_type::if_);
                else tokens.emplace_back(token_type::identifier_, token_value);
            }
            else if (try_consume_token('(')) tokens.emplace_back(token_type::open_parenthesis_);
            else if (try_consume_token(')')) tokens.emplace_back(token_type::close_parenthesis_);
            else if (try_consume_token('{')) tokens.emplace_back(token_type::open_brace_);
            else if (try_consume_token('}')) tokens.emplace_back(token_type::close_brace_);
            else if (try_consume_token('[')) tokens.emplace_back(token_type::open_bracket_);
            else if (try_consume_token(']')) tokens.emplace_back(token_type::close_bracket_);
            else if (try_consume_token(':')) tokens.emplace_back(token_type::colon_);
            else if (try_consume_token(';')) tokens.emplace_back(token_type::semicolon_);
            else if (try_consume_token(',')) tokens.emplace_back(token_type::comma_);
            else if (try_consume_token('=')) tokens.emplace_back(token_type::equals_);

            else if (try_consume_token('/')) tokens.emplace_back(token_type::forward_slash_);
            else if (try_consume_token('%')) tokens.emplace_back(token_type::percent_);
            else if (try_consume_token('*')) tokens.emplace_back(token_type::asterisk_);
            else if (try_consume_token('+')) tokens.emplace_back(token_type::plus_);
            else if (try_consume_token('-')) tokens.emplace_back(token_type::minus_);

            else error_exit("Invalid character.");
        }

        reset();
        return tokens;
    }

    bool tokenizer::try_consume_whitespace_and_comments()
    {
        if (try_consume_token(&is_space, &is_space))
            return true;
        if (try_consume_token("//"))
        {
            // Consume the rest of the line.
            (void)try_consume_token(&is_not_line_ending, &is_not_line_ending);
            // Consume the line ending.
            (void)try_consume_token(&is_space, &is_space);
            return true;
        }
        if (try_consume_token("/*"))
        {
            char looking_for = '*';
            while (auto c = peek())
            {
                consume();
                if (*c == looking_for)
                {
                    if (looking_for == '*')
                        looking_for = '/';
                    else // the end of the multi-line comment has been found
                        return true;
                }
                else // reset what we're looking for.
                    looking_for = '*';
            }
            // there is no end to the multi-line comment.
            error_exit("Multi-line comment never closed. Expected \"*/\".");
        }
        return false;
    }

    std::optional<std::string_view> tokenizer::try_consume_integer_literal()
    {
        return try_consume_token(&is_digit_10, &is_digit_10);
    }

    std::optional<std::string_view> tokenizer::try_consume_identifier()
    {
        return try_consume_token(&is_identifier_first, &is_identifier_rest);
    }

    std::optional<std::string_view> tokenizer::try_consume_token(token_pred first_pred, token_pred rest_pred)
    {
        if (first_pred(*peek())) // guaranteed to be valid.
        {
            std::size_t i = 1;
            for (auto c = peek(i); c && rest_pred(*c); ++i, c = peek(i));

            auto token_begin_it = consume(i);
            auto token_end_it = iterator();
            return std::string_view(token_begin_it, token_end_it);
        }
        return std::nullopt;
    }

    std::optional<std::string_view> tokenizer::try_consume_token(token_pred char_pred)
    {
        if (char_pred(*peek())) // guaranteed to be valid.
        {
            auto token_begin_it = consume();
            auto token_end_it = iterator();
            return std::string_view(token_begin_it, token_end_it);
        }
        return std::nullopt;
    }

    std::optional<std::string_view> tokenizer::try_consume_token(std::string_view wanted)
    {
        std::size_t i = 0;
        for (auto c = peek(i); c && i < wanted.size() && c == wanted[i]; ++i, c = peek(i));
        if (i == wanted.size())
        {
            auto token_begin_it = consume(i);
            auto token_end_it = iterator();
            return std::string_view(token_begin_it, token_end_it);
        }
        return std::nullopt;
    }

    std::optional<std::string_view> tokenizer::try_consume_token(char wanted)
    {
        static thread_local char c_wanted; // thread_local for future proofing.
        c_wanted = wanted;
        return try_consume_token([](char c) { return c == c_wanted; });
    }
} // namespace shl
