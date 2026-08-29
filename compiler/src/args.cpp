#include "args.hpp"
#include "debug.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void print_syntax(const char* program)
{
    std::cerr << "Syntax: " << program << " [-flags] <filepath>\n";
}

void print_valid_flags()
{
    std::cerr << "Valid flags:\n"
              << "  -d    Debug mode\n";
}

} // namespace

Arguments parse_arguments(int argc, char* argv[])
{
    if (argc < 2 || argc > 3) {
        print_syntax(argv[0]);
        throw std::runtime_error("invalid argument count");
    }

    Arguments arguments;
    const std::string first_argument = argv[1];

    // No flag: argv[1] is the filepath.
    if (first_argument.empty() || first_argument[0] != '-') {
        if (argc != 2) {
            print_syntax(argv[0]);
            throw std::runtime_error("invalid argument count");
        }

        arguments.filepath = first_argument;
        csam_debug = false;
        return arguments;
    }

    // Flag mode: argv[1] contains flags and argv[2] is the filepath.
    if (argc != 3) {
        print_syntax(argv[0]);
        throw std::runtime_error("missing filepath");
    }

    for (std::size_t i = 1; i < first_argument.size(); ++i) {
        switch (first_argument[i]) {
            case 'd':
                arguments.debug = true;
                break;

            default:
                std::cerr << "Invalid flag: -" << first_argument[i] << '\n';
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
