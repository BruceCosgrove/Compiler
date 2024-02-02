#include "input.hpp"
#include "ctype.hpp"
#include "error.hpp"
#include "util.hpp"
#include <iostream>
#include <getopt.h> // https://linux.die.net/man/3/optarg

namespace shl
{
    static input _input;

    const input& handle_input(int argc, char *argv[])
    {
        // All options that require an argument.
        static const std::string opts_r = insert_after_each("ove", ":");
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
                try
                {
                    _input.verbose_level = static_cast<decltype(input::verbose_level)>(std::stoi(optarg));
                    if (_input.verbose_level >= input::verbose_level::_count)
                        error_exit("Input", "Invalid -v argument");
                }
                catch (std::exception&) // The stl has it's ugly parts too.
                {
                    error_exit("Input", "Invalid -v argument");
                }
                break;
            case 'e':
                _input.entry_point = optarg; // Construct std::string_view once (std::strlen called).
                if (!is_identifier(_input.entry_point))
                    error_exit("Input", "Invalid -e argument: it's not an identifier");
                break;
            }
        }

        // TODO: For now, only accept one input file.
        if (optind + 1 != argc)
            error_exit("Input", "You must provide exactly one input source file");

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
