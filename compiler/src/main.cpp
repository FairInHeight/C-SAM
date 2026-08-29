#include "debug.hpp"
#include "lexer.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

int main()
{
    std::ifstream file("test.csam");

    if (!file) {
        std::cerr << "Could not open test.csam\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    std::string source = buffer.str();

    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

    if (csam_debug) {
        for (const Token& token : tokens) {
            std::cout
                << token.line << ":"
                << token.column << " "
                << token_type_name(token.type);

            if (!token.value.empty()) {
                std::cout << " \"" << token.value << "\"";
            }

            std::cout << '\n';
        }
    }

    return 0;
}