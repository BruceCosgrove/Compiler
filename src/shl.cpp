#include "error.hpp"
#include "fileio.hpp"
#include "generator.hpp"
#include "parser.hpp"
#include "tokenizer.hpp"
#include <filesystem>
#include <iostream> // DEBUG
// #include <getopt.h> // https://linux.die.net/man/3/optarg

using namespace shl;

struct input
{
    std::filesystem::path in_path;
    std::filesystem::path out_path;
};

const input& handle_input(int argc, char* argv[])
{
    static input input;

    if (!(2 <= argc && argc <= 3))
        error_exit("Incorrect usage. Proper usage is \"shl <input.shl> [output.asm]\".");

    input.in_path = argv[1];
    if (argc == 2)
        input.out_path = input.in_path.replace_extension("asm");
    else
        input.out_path = argv[2];

    return input;
}

int main(int argc, char* argv[])
{
    auto& input = handle_input(argc, argv);

    // Read the input file.
    std::string in_file_contents;
    if (!fileio::read(input.in_path, in_file_contents))
        error_exit("Unable to open input file.");

    auto assembly = generator(parser(tokenizer(std::move(in_file_contents))())())();

    // Write the output file.
    if (!fileio::write(input.out_path, assembly))
        error_exit("Unable to open output file.");

    return EXIT_SUCCESS;
}
