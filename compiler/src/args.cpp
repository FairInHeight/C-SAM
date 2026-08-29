#include "args.hpp"
#include "debug.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void print_syntax(const char* program)
{
    std::cerr << "Syntax: " << program << " [-flags] <filepath> [filepath ...]\n";
}

void print_valid_flags()
{
    std::cerr << "Valid flags:\n"
              << "  -d    Debug mode\n";
}

} // namespace

Arguments parse_arguments(int argc, char* argv[])
{
    if (argc < 2) {
        print_syntax(argv[0]);
        throw std::runtime_error("missing filepath");
    }

    Arguments arguments;
    const std::string first_argument = argv[1];
    int first_filepath = 1;

    // No flags: argv[1] is the first filepath and every following argument
    // is an additional filepath.
    if (first_argument.empty() || first_argument[0] != '-') {
        first_filepath = 1;
    } else {
        // Flags are restricted to argv[1]. All following arguments are paths.
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

        first_filepath = 2;

        if (argc < 3) {
            print_syntax(argv[0]);
            throw std::runtime_error("missing filepath");
        }
    }

    for (int i = first_filepath; i < argc; ++i) {
        if (std::string(argv[i]).empty()) {
            print_syntax(argv[0]);
            throw std::runtime_error("empty filepath");
        }

        arguments.filepaths.emplace_back(argv[i]);
    }

    csam_debug = arguments.debug;

    return arguments;
}
