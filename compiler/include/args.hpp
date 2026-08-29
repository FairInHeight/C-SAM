#ifndef CSAM_ARGS_HPP
#define CSAM_ARGS_HPP

#include <string>
#include <vector>

struct Arguments {
    bool debug = false;
    std::vector<std::string> filepaths;
};

Arguments parse_arguments(int argc, char* argv[]);

#endif
