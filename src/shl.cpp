#include "error.hpp"
#include "fileio.hpp"
#include "tokenizer.hpp"
#include <filesystem>
#include <iostream>
#include <format>
#include <getopt.h> // https://linux.die.net/man/3/optarg

using namespace shl;

int main(int argc, char* argv[])
{
    if (!(2 <= argc && argc <= 3))
        error_exit("Incorrect usage. Proper usage is \"shl <input.shl> [output.asm]\".");

    std::filesystem::path in_path = argv[1];
    std::filesystem::path out_path;
    if (argc == 2)
        out_path = in_path.replace_extension("asm");
    else
        out_path = argv[2];

    std::string in_file_contents;
    if (!fileio::read(in_path, in_file_contents))
        error_exit("Unable to open input file.");

    tokenizer tokenizer(std::move(in_file_contents));
    auto tokens = tokenizer.tokenize();
    for (auto& token : tokens)
    {
        std::cout << +token.type;
        if (token.value)
            std::cout << ": \"" << token.value.value() << '"';
        std::cout << '\n';
    }

    return EXIT_SUCCESS;
}
