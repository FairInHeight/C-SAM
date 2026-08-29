#include "lexer.hpp"
#include "debug.hpp"

#include <cctype>
#include <iostream>
#include <stdexcept>

Lexer::Lexer(const std::string& source)
    : source(source)
{
}

std::vector<Token> Lexer::tokenize()
{
    if (csam_debug) {
        std::cout << "Lexer: Tokenizing source\n";
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

        if (current == '/' &&
            position + 1 < source.size() &&
            source[position + 1] == '/') {

            position += 2;
            column += 2;

            while (position < source.size() && source[position] != '\n') {
                ++position;
                ++column;
            }

            continue;
        }

        if (current == '/' &&
            position + 1 < source.size() &&
            source[position + 1] == '*') {

            position += 2;
            column += 2;

            while (position < source.size()) {
                if (source[position] == '*' &&
                    position + 1 < source.size() &&
                    source[position + 1] == '/') {

                    position += 2;
                    column += 2;
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

            continue;
        }

        if (std::isalpha(static_cast<unsigned char>(current)) ||
            current == '_') {

            std::size_t start = position;
            std::size_t startColumn = column;

            while (position < source.size() &&
                   (std::isalnum(static_cast<unsigned char>(source[position])) ||
                    source[position] == '_' ||
                    source[position] == '-')) {

                ++position;
                ++column;
            }

            tokens.push_back({
                TokenType::Identifier,
                source.substr(start, position - start),
                line,
                startColumn
            });

            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(current))) {
            std::size_t start = position;
            std::size_t startColumn = column;

            while (position < source.size() &&
                   (std::isdigit(static_cast<unsigned char>(source[position])) ||
                    source[position] == '.')) {

                ++position;
                ++column;
            }

            tokens.push_back({
                TokenType::Number,
                source.substr(start, position - start),
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

            while (position < source.size() && source[position] != '"') {
                if (source[position] == '\n') {
                    ++line;
                    column = 1;
                } else {
                    ++column;
                }

                ++position;
            }

            if (position >= source.size()) {
                throw std::runtime_error("Unterminated string");
            }

            ++position;
            ++column;

            tokens.push_back({
                TokenType::String,
                source.substr(start, position - start),
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
                    source[position] == '_' ||
                    source[position] == '-')) {

                ++position;
                ++column;
            }

            tokens.push_back({
                TokenType::Hash,
                source.substr(start, position - start),
                line,
                startColumn
            });

            continue;
        }

        TokenType type;

        switch (current) {
            case ':':
                type = TokenType::Colon;
                break;

            case ';':
                type = TokenType::Semicolon;
                break;

            case '{':
                type = TokenType::LeftBrace;
                break;

            case '}':
                type = TokenType::RightBrace;
                break;

            case '=':
                type = TokenType::Equals;
                break;

            case ',':
                type = TokenType::Comma;
                break;

            default:
                throw std::runtime_error(
                    "Unexpected character at line " +
                    std::to_string(line) +
                    ", column " +
                    std::to_string(column)
                );
        }

        tokens.push_back({
            type,
            std::string(1, current),
            line,
            column
        });

        ++position;
        ++column;
    }

    tokens.push_back({
        TokenType::EndOfFile,
        "",
        line,
        column
    });

    if (csam_debug) {
        std::cout << "Lexer: Finished tokenizing source\n";
    }

    return tokens;
}