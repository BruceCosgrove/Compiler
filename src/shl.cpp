#include "error.hpp"
#include "fileio.hpp"
#include "input.hpp"
#include "front/lexer.hpp"
#include "front/parser.hpp"
#include "back/generator.hpp"

using namespace shl;

int main(int argc, char* argv[])
{
    auto& input = handle_input(argc, argv);

    // Read the input file.
    std::string in_file_contents;
    if (!fileio::read(input.in_path, in_file_contents))
        error_exit("Input", "Unable to open input file");

    lexer lexer(std::move(in_file_contents));
    parser parser(lexer());
    generator generator(parser());
    auto assembly = generator();

    // Write the output file.
    if (!fileio::write(input.out_path, assembly))
        error_exit("Output", "Unable to open output file");

    return EXIT_SUCCESS;
}
