#include "tokenizer.hpp"
#include "error.hpp"
#include <cctype>
#include <iostream> // DEBUG

namespace shl
{
    std::vector<token> tokenizer::tokenize()
    {
        std::vector<token> tokens;

        size_type token_begin_index = 0;
        size_type token_end_index = 0;

        while (auto c = peek())
        {
            if (std::isspace(c.value()))
            {
                consume();
                continue;
            }
            if (std::isalpha(c.value()))
            {
                token_begin_index = index();
                consume();
                token_end_index = index();

                for (c = peek(); c && std::isalnum(c.value()); c = peek())
                {
                    ++token_end_index;
                    consume();
                }

                auto token_value = std::string_view(container()).substr(token_begin_index, token_end_index - token_begin_index);
                if (token_value == "return") tokens.emplace_back(token_type::_return);
                else error_exit("Invalid identifier.");
            }
            else if (std::isdigit(c.value()))
            {
                token_begin_index = index();
                consume();
                token_end_index = index();

                while ((c = peek()) && std::isdigit(c.value()))
                {
                    ++token_end_index;
                    consume();
                }

                auto token_value = std::string_view(container()).substr(token_begin_index, token_end_index - token_begin_index);
                tokens.emplace_back(token_type::_integer_literal, token_value);
            }
            else if (c == ';')
            {
                // token_begin_index = index();
                consume();
                // token_end_index = index();

                // auto token_value = std::string_view(container()).substr(token_begin_index, token_end_index - token_begin_index);
                tokens.emplace_back(token_type::_semicolon);
            }
            else error_exit("Invalid character.");
        }

        return tokens;
    }
} // namespace shl
