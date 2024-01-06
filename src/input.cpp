#include "input.hpp"
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

    void handle_input(int argc, char *argv[])
    {
        // All options that require an argument.
        static const std::string opts_r = insert_after_each("o", ':');
        // All options that have no arguments.
        static const std::string opts = "v";
        // The full option string.
        static const std::string opts_all = opts_r + opts;

        for (int c; (c = getopt(argc, argv, opts_all.c_str())) != -1; )
        {
            switch (c)
            {
                case 'o':
                    _input.out_path = optarg;
                    break;
                case 'v':
                    _input.verbose = true;
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
    }

    const input& get_input() noexcept
    {
        return _input;
    }

} // namespace shl
