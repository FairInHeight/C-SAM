#include "args.hpp"
#include "debug.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void print_syntax(const char* program)
{
    std::cerr << "Syntax: " << program << " -[flags] <filepath>\n";
}

void print_valid_flags()
{
    std::cerr << "Valid flags:\n"
              << "  -d    Debug mode\n";
}

} // namespace

Arguments parse_arguments(int argc, char* argv[])
{
    if (argc < 3 || argc > 3) {
        print_syntax(argv[0]);
        throw std::runtime_error("invalid argument count");
    }

    const std::string flags = argv[1];

    if (flags.empty() || flags[0] != '-') {
        print_syntax(argv[0]);
        throw std::runtime_error("invalid flag syntax");
    }

    Arguments arguments;

    for (std::size_t i = 1; i < flags.size(); ++i) {
        switch (flags[i]) {
            case 'd':
                arguments.debug = true;
                break;

            default:
                std::cerr << "Invalid flag: -" << flags[i] << '\n';
                print_valid_flags();
                throw std::invalid_argument("invalid flag");
        }
    }

    arguments.filepath = argv[2];

    if (arguments.filepath.empty()) {
        print_syntax(argv[0]);
        throw std::runtime_error("missing filepath");
    }

    csam_debug = arguments.debug;

    return arguments;
}
