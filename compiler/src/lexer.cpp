#include "lexer.hpp"
#include "debug.hpp"

#include <cctype>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

Lexer::Lexer(const std::string& source, const std::filesystem::path& filepath)
    : source(source), filepath(filepath)
{
}

std::vector<Token> Lexer::tokenize()
{
    if (csam_debug) {
        std::cout << "Lexer: Tokenizing " << filepath.string() << '\n';
    }

    std::vector<Token> tokens;

    std::size_t position = 0;
    std::size_t line = 1;
    std::size_t column = 1;

    while (position < source.size()) {
        const char current = source[position];

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
            const std::size_t comment_line = line;
            const std::size_t comment_column = column;

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
                    filepath.string() + ":" + std::to_string(comment_line) + ":" +
                    std::to_string(comment_column) + ": Unterminated comment"
                );
            }

            continue;
        }

        if (std::isalpha(static_cast<unsigned char>(current)) || current == '_') {
            const std::size_t start = position;
            const std::size_t start_column = column;

            while (position < source.size() &&
                   (std::isalnum(static_cast<unsigned char>(source[position])) ||
                    source[position] == '_' || source[position] == '-')) {
                ++position;
                ++column;
            }

            tokens.push_back({
                TokenType::Identifier,
                source.substr(start, position - start),
                {filepath, line, start_column}
            });

            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(current))) {
            const std::size_t start = position;
            const std::size_t start_column = column;

            while (position < source.size() &&
                   (std::isdigit(static_cast<unsigned char>(source[position])) || source[position] == '.')) {
                ++position;
                ++column;
            }

            tokens.push_back({
                TokenType::Number,
                source.substr(start, position - start),
                {filepath, line, start_column}
            });

            continue;
        }

        if (current == '"') {
            const std::size_t start = position;
            const std::size_t start_line = line;
            const std::size_t start_column = column;

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
                    filepath.string() + ":" + std::to_string(start_line) + ":" +
                    std::to_string(start_column) + ": Unterminated string"
                );
            }

            tokens.push_back({
                TokenType::String,
                source.substr(start, position - start),
                {filepath, start_line, start_column}
            });

            continue;
        }

        if (current == '#') {
            const std::size_t start = position;
            const std::size_t start_column = column;

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
                {filepath, line, start_column}
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
                    filepath.string() + ":" + std::to_string(line) + ":" +
                    std::to_string(column) + ": Unexpected character"
                );
        }

        tokens.push_back({
            type,
            std::string(1, current),
            {filepath, line, column}
        });

        ++position;
        ++column;
    }

    tokens.push_back({
        TokenType::EndOfFile,
        "",
        {filepath, line, column}
    });

    if (csam_debug) {
        std::cout << "Lexer: Finished tokenizing " << filepath.string() << '\n';
    }

    return tokens;
}
