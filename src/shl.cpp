#include "error.hpp"
#include "fileio.hpp"
#include "generator.hpp"
#include "input.hpp"
#include "parser.hpp"
#include "tokenizer.hpp"

using namespace shl;

int main(int argc, char* argv[])
{
    auto& input = handle_input(argc, argv);

    // Read the input file.
    std::string in_file_contents;
    if (!fileio::read(input.in_path, in_file_contents))
        error_exit("Unable to open input file.");

    tokenizer tokenizer(std::move(in_file_contents));
    parser parser(tokenizer());
    generator generator(parser());
    auto assembly = generator();

    // Write the output file.
    if (!fileio::write(input.out_path, assembly))
        error_exit("Unable to open output file.");

    return EXIT_SUCCESS;
}
