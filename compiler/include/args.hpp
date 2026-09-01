#ifndef CSAM_ARGS_HPP
#define CSAM_ARGS_HPP

#include <filesystem>
#include <string>
#include <vector>

struct Arguments {
    bool debug = false;
    std::vector<std::filesystem::path> filepaths;
};

Arguments parse_arguments(int argc, char* argv[]);

#endif
