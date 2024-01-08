#include "input.hpp"
#include "ctype.hpp"
#include "error.hpp"
#include <iostream>
#include <getopt.h> // https://linux.die.net/man/3/optarg

namespace shl
{
    // Util function.
    std::string insert_after_each(std::string_view string, char after)
    {
        std::string result;
        result.reserve(string.size() * 2);
        for (char c : string)
            (result += c) += after;
        return result;
    }

    static input _input;

    const input& handle_input(int argc, char *argv[])
    {
        // All options that require an argument.
        static const std::string opts_r = insert_after_each("ove", ':');
        // All options that have no arguments.
        static const std::string opts_n = "";
        // The full option string.
        static const std::string opts_all = opts_r + opts_n;

        for (int c; (c = getopt(argc, argv, opts_all.c_str())) != -1; )
        {
            switch (c)
            {
                case 'o':
                    _input.out_path = optarg;
                    break;
                case 'v':
                    _input.verbose_level = static_cast<std::uint8_t>(std::atoi(optarg));
                    break;
                case 'e':
                    if (!is_identifier_first(*optarg))
                        error_exit("Invalid -e argument: it's not an identifier.");
                    for (std::size_t i = 0; optarg[i]; ++i)
                        if (!is_identifier_rest(*optarg))
                            error_exit("Invalid -e argument: it's not an identifier.");
                    _input.entry_point = optarg;
                    break;
            }
        }

        // TODO: For now, only accept one input file.
        if (optind + 1 != argc)
            error_exit("You must provide exactly one input source file.");

        _input.in_path = argv[optind];
        if (_input.out_path.empty())
        {
            _input.out_path = _input.in_path;
            _input.out_path.replace_extension("asm");
        }

        return _input;
    }

    const input& get_input() noexcept
    {
        return _input;
    }

} // namespace shl
