#include "tokenizer.hpp"
#include "error.hpp"
#include "ctype.hpp"

namespace shl
{
    std::vector<token> tokenizer::operator()()
    {
        std::vector<token> tokens;

        while (peek())
        {
            if (try_consume_whitespace_and_comments()) continue;
            else if (auto token_value = try_consume_integer_literal())
                tokens.emplace_back(token_type::integer_literal_, line_number, get_column_count(), *token_value);
            else if (auto token_value = try_consume_identifier())
            {
                     if (token_value == "return") tokens.emplace_back(token_type::return_, line_number, get_column_count());
                else if (token_value == "if")     tokens.emplace_back(token_type::if_, line_number, get_column_count());
                else if (token_value == "elif")   tokens.emplace_back(token_type::elif_, line_number, get_column_count());
                else if (token_value == "else")   tokens.emplace_back(token_type::else_, line_number, get_column_count());
                else tokens.emplace_back(token_type::identifier_, line_number, get_column_count(), *token_value);
            }
            else if (try_consume_token('(')) tokens.emplace_back(token_type::open_parenthesis_, line_number, get_column_count());
            else if (try_consume_token(')')) tokens.emplace_back(token_type::close_parenthesis_, line_number, get_column_count());
            else if (try_consume_token('{')) tokens.emplace_back(token_type::open_brace_, line_number, get_column_count());
            else if (try_consume_token('}')) tokens.emplace_back(token_type::close_brace_, line_number, get_column_count());
            else if (try_consume_token('[')) tokens.emplace_back(token_type::open_bracket_, line_number, get_column_count());
            else if (try_consume_token(']')) tokens.emplace_back(token_type::close_bracket_, line_number, get_column_count());
            else if (try_consume_token(':')) tokens.emplace_back(token_type::colon_, line_number, get_column_count());
            else if (try_consume_token(';')) tokens.emplace_back(token_type::semicolon_, line_number, get_column_count());
            else if (try_consume_token(',')) tokens.emplace_back(token_type::comma_, line_number, get_column_count());
            else if (try_consume_token('=')) tokens.emplace_back(token_type::equals_, line_number, get_column_count());

            else if (try_consume_token('/')) tokens.emplace_back(token_type::forward_slash_, line_number, get_column_count());
            else if (try_consume_token('%')) tokens.emplace_back(token_type::percent_, line_number, get_column_count());
            else if (try_consume_token('*')) tokens.emplace_back(token_type::asterisk_, line_number, get_column_count());
            else if (try_consume_token('+')) tokens.emplace_back(token_type::plus_, line_number, get_column_count());
            else if (try_consume_token('-')) tokens.emplace_back(token_type::minus_, line_number, get_column_count());

            else error_exit("Tokenizer", "Invalid character", line_number, get_column_count());
        }

        reset();
        line_number = 1;
        return tokens;
    }

    bool tokenizer::try_consume_whitespace_and_comments()
    {
        if (*peek() == '\n')
        {
            consume();
            on_newline();
            return true;
        }
        if (try_consume_token(&is_space, &is_space))
            return true;
        if (try_consume_token("//"))
        {
            // Consume the rest of the line.
            (void)try_consume_token(&is_not_line_ending, &is_not_line_ending);
            // Consume the line ending.
            if (try_consume_token(&is_space, &is_space))
                on_newline();
            return true;
        }
        if (try_consume_token("/*"))
        {
            char looking_for = '*';
            while (auto c = peek())
            {
                if (*c == '\n')
                    on_newline();
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
            error_exit("Tokenizer", "Multi-line comment never closed. Expected \"*/\"", line_number, get_column_count());
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

    void tokenizer::on_newline()
    {
        ++line_number;
        column_iterator = iterator();
    }

    std::uint32_t tokenizer::get_column_count()
    {
        return iterator() - column_iterator;
    }
} // namespace shl
