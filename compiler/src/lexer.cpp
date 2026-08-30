#include "lexer.hpp"
#include "debug.hpp"

#include <cctype>
#include <iostream>
#include <stdexcept>
#include <vector>

Lexer::Lexer(const std::string& source, const std::string& filepath)
    : source(source), filepath(filepath)
{
}

std::vector<Token> Lexer::tokenize()
{
    if (csam_debug) {
        std::cout << "Lexer: Tokenizing " << filepath << '\n';
    }

    std::vector<Token> tokens;

    std::size_t position = 0;
    std::size_t line = 1;
    std::size_t column = 1;

    while (position < source.size()) {
        char current = source[position];

        if (std::isspace(static_cast<unsigned char>(current))) {
            if (current == '\n') {
                ++line;
                column = 1;
            } else {
                ++column;
            }

            ++position;
            continue;
        }

        if (current == '/' && position + 1 < source.size() && source[position + 1] == '/') {
            position += 2;
            column += 2;

            while (position < source.size() && source[position] != '\n') {
                ++position;
                ++column;
            }

            continue;
        }

        if (current == '/' && position + 1 < source.size() && source[position + 1] == '*') {
            const std::size_t commentLine = line;
            const std::size_t commentColumn = column;

            position += 2;
            column += 2;
            bool closed = false;

            while (position < source.size()) {
                if (source[position] == '*' && position + 1 < source.size() && source[position + 1] == '/') {
                    position += 2;
                    column += 2;
                    closed = true;
                    break;
                }

                if (source[position] == '\n') {
                    ++line;
                    column = 1;
                } else {
                    ++column;
                }

                ++position;
            }

            if (!closed) {
                throw std::runtime_error(
                    filepath + ":" + std::to_string(commentLine) + ":" +
                    std::to_string(commentColumn) + ": Unterminated comment"
                );
            }

            continue;
        }

        if (std::isalpha(static_cast<unsigned char>(current)) || current == '_') {
            std::size_t start = position;
            std::size_t startColumn = column;

            while (position < source.size() &&
                   (std::isalnum(static_cast<unsigned char>(source[position])) ||
                    source[position] == '_' || source[position] == '-')) {
                ++position;
                ++column;
            }

            tokens.push_back({
                TokenType::Identifier,
                source.substr(start, position - start),
                filepath,
                line,
                startColumn
            });

            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(current))) {
            std::size_t start = position;
            std::size_t startColumn = column;

            while (position < source.size() &&
                   (std::isdigit(static_cast<unsigned char>(source[position])) || source[position] == '.')) {
                ++position;
                ++column;
            }

            tokens.push_back({
                TokenType::Number,
                source.substr(start, position - start),
                filepath,
                line,
                startColumn
            });

            continue;
        }

        if (current == '"') {
            std::size_t start = position;
            std::size_t startLine = line;
            std::size_t startColumn = column;

            ++position;
            ++column;

            bool closed = false;

            while (position < source.size()) {
                if (source[position] == '"') {
                    ++position;
                    ++column;
                    closed = true;
                    break;
                }

                if (source[position] == '\n') {
                    ++line;
                    column = 1;
                } else {
                    ++column;
                }

                ++position;
            }

            if (!closed) {
                throw std::runtime_error(
                    filepath + ":" + std::to_string(startLine) + ":" +
                    std::to_string(startColumn) + ": Unterminated string"
                );
            }

            tokens.push_back({
                TokenType::String,
                source.substr(start, position - start),
                filepath,
                startLine,
                startColumn
            });

            continue;
        }

        if (current == '#') {
            std::size_t start = position;
            std::size_t startColumn = column;

            ++position;
            ++column;

            while (position < source.size() &&
                   (std::isalnum(static_cast<unsigned char>(source[position])) ||
                    source[position] == '_' || source[position] == '-')) {
                ++position;
                ++column;
            }

            tokens.push_back({
                TokenType::Hash,
                source.substr(start, position - start),
                filepath,
                line,
                startColumn
            });

            continue;
        }

        TokenType type;

        switch (current) {
            case ':': type = TokenType::Colon; break;
            case ';': type = TokenType::Semicolon; break;
            case '{': type = TokenType::LeftBrace; break;
            case '}': type = TokenType::RightBrace; break;
            case '<': type = TokenType::LeftAngle; break;
            case '>': type = TokenType::RightAngle; break;
            case '[': type = TokenType::LeftBracket; break;
            case ']': type = TokenType::RightBracket; break;
            case '(': type = TokenType::LeftParen; break;
            case ')': type = TokenType::RightParen; break;
            case '=': type = TokenType::Equals; break;
            case ',': type = TokenType::Comma; break;
            default:
                throw std::runtime_error(
                    filepath + ":" + std::to_string(line) + ":" +
                    std::to_string(column) + ": Unexpected character"
                );
        }

        tokens.push_back({
            type,
            std::string(1, current),
            filepath,
            line,
            column
        });

        ++position;
        ++column;
    }

    tokens.push_back({
        TokenType::EndOfFile,
        "",
        filepath,
        line,
        column
    });

    if (csam_debug) {
        std::cout << "Lexer: Finished tokenizing " << filepath << '\n';
    }

    return tokens;
}