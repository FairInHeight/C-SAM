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

    for (const Token& token : tokens) {
        std::cout
            << token.line << ":"
            << token.column << " ";

        switch (token.type) {
            case TokenType::Identifier:
                std::cout << "Identifier";
                break;

            case TokenType::String:
                std::cout << "String";
                break;

            case TokenType::Number:
                std::cout << "Number";
                break;

            case TokenType::Colon:
                std::cout << "Colon";
                break;

            case TokenType::Semicolon:
                std::cout << "Semicolon";
                break;

            case TokenType::LeftBrace:
                std::cout << "LeftBrace";
                break;

            case TokenType::RightBrace:
                std::cout << "RightBrace";
                break;

            case TokenType::Hash:
                std::cout << "Hash";
                break;

            case TokenType::Comma:
                std::cout << "Comma";
                break;

            case TokenType::EndOfFile:
                std::cout << "EndOfFile";
                break;
        }

        if (!token.value.empty()) {
            std::cout << " \"" << token.value << "\"";
        }

        std::cout << '\n';
    }

    return 0;
}