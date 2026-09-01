#ifndef CSAM_LEXER_HPP
#define CSAM_LEXER_HPP

#include "token.hpp"

#include <filesystem>
#include <string>
#include <vector>

class Lexer {
public:
    Lexer(const std::string& source, const std::filesystem::path& filepath);

    std::vector<Token> tokenize();

private:
    const std::string& source;
    std::filesystem::path filepath;
};

#endif
