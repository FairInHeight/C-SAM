#ifndef CSAM_ARGS_HPP
#define CSAM_ARGS_HPP

#include <string>

struct Arguments {
    bool debug = false;
    std::string filepath;
};

Arguments parse_arguments(int argc, char* argv[]);

#endif
