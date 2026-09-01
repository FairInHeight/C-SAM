#include "lexer.hpp"
#include "debug.hpp"

#include <cctype>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

bool is_ascii_letter(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

bool is_hex_digit(char c)
{
    return (c >= '0' && c <= '9') ||
           (c >= 'A' && c <= 'F') ||
           (c >= 'a' && c <= 'f');
}

bool is_css_whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f';
}

bool decode_utf8(const std::string& source, std::size_t position, std::uint32_t& codepoint, std::size_t& length)
{
    if (position >= source.size()) {
        return false;
    }

    const unsigned char first = static_cast<unsigned char>(source[position]);

    if (first < 0x80) {
        codepoint = first;
        length = 1;
        return true;
    }

    if (first >= 0xC2 && first <= 0xDF) {
        if (position + 1 >= source.size()) return false;
        const unsigned char b1 = static_cast<unsigned char>(source[position + 1]);
        if ((b1 & 0xC0) != 0x80) return false;
        codepoint = ((first & 0x1F) << 6) | (b1 & 0x3F);
        length = 2;
        return true;
    }

    if (first >= 0xE0 && first <= 0xEF) {
        if (position + 2 >= source.size()) return false;
        const unsigned char b1 = static_cast<unsigned char>(source[position + 1]);
        const unsigned char b2 = static_cast<unsigned char>(source[position + 2]);
        if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80) return false;
        if (first == 0xE0 && b1 < 0xA0) return false;
        if (first == 0xED && b1 >= 0xA0) return false;
        codepoint = ((first & 0x0F) << 12) | ((b1 & 0x3F) << 6) | (b2 & 0x3F);
        length = 3;
        return true;
    }

    if (first >= 0xF0 && first <= 0xF4) {
        if (position + 3 >= source.size()) return false;
        const unsigned char b1 = static_cast<unsigned char>(source[position + 1]);
        const unsigned char b2 = static_cast<unsigned char>(source[position + 2]);
        const unsigned char b3 = static_cast<unsigned char>(source[position + 3]);
        if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80 || (b3 & 0xC0) != 0x80) return false;
        if (first == 0xF0 && b1 < 0x90) return false;
        if (first == 0xF4 && b1 > 0x8F) return false;
        codepoint = ((first & 0x07) << 18) | ((b1 & 0x3F) << 12) |
                    ((b2 & 0x3F) << 6) | (b3 & 0x3F);
        length = 4;
        return true;
    }

    return false;
}

bool is_non_ascii_codepoint(std::uint32_t codepoint)
{
    return codepoint >= 0x80 && codepoint <= 0x10FFFF;
}

bool is_name_start(const std::string& source, std::size_t position)
{
    if (position >= source.size()) return false;

    const unsigned char c = static_cast<unsigned char>(source[position]);
    if (is_ascii_letter(static_cast<char>(c)) || c == '_') return true;

    std::uint32_t codepoint = 0;
    std::size_t length = 0;
    return c >= 0x80 && decode_utf8(source, position, codepoint, length) &&
           is_non_ascii_codepoint(codepoint);
}

bool is_name_codepoint(const std::string& source, std::size_t position)
{
    if (position >= source.size()) return false;

    const unsigned char c = static_cast<unsigned char>(source[position]);
    if (is_name_start(source, position) || (c >= '0' && c <= '9') || c == '-') return true;

    return false;
}

bool is_valid_escape(const std::string& source, std::size_t position)
{
    if (position >= source.size() || source[position] != '\\' || position + 1 >= source.size()) {
        return false;
    }

    const char next = source[position + 1];
    return next != '\n' && next != '\r' && next != '\f';
}

bool starts_identifier(const std::string& source, std::size_t position)
{
    if (position >= source.size()) return false;

    const char current = source[position];

    if (current == '\\') return is_valid_escape(source, position);
    if (is_name_start(source, position)) return true;

    if (current == '-') {
        if (position + 1 >= source.size()) return false;
        return is_name_start(source, position + 1) ||
               source[position + 1] == '-' ||
               source[position + 1] == '\\';
    }

    return false;
}

bool starts_number(const std::string& source, std::size_t position)
{
    if (position >= source.size()) return false;

    std::size_t index = position;
    if (source[index] == '+' || source[index] == '-') {
        ++index;
        if (index >= source.size()) return false;
    }

    if (std::isdigit(static_cast<unsigned char>(source[index]))) return true;

    return source[index] == '.' && index + 1 < source.size() &&
           std::isdigit(static_cast<unsigned char>(source[index + 1]));
}

void consume_escape(const std::string& source, std::size_t& position, std::size_t& column)
{
    if (!is_valid_escape(source, position)) {
        throw std::runtime_error("Invalid CSS escape");
    }

    ++position;
    ++column;

    if (is_hex_digit(source[position])) {
        std::size_t digits = 0;
        while (position < source.size() && digits < 6 && is_hex_digit(source[position])) {
            ++position;
            ++column;
            ++digits;
        }

        if (position < source.size() && is_css_whitespace(source[position])) {
            ++position;
            ++column;
        }
    } else {
        ++position;
        ++column;
    }
}

} // namespace

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

    auto add_token = [&](TokenType type, std::size_t start, std::size_t start_line, std::size_t start_column) {
        tokens.push_back({type, source.substr(start, position - start), {filepath, start_line, start_column}});
    };

    while (position < source.size()) {
        const char current = source[position];

        if (is_css_whitespace(current)) {
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
                    std::to_string(comment_column) + ": Unterminated comment");
            }
            continue;
        }

        if (starts_number(source, position)) {
            const std::size_t start = position;
            const std::size_t start_line = line;
            const std::size_t start_column = column;

            if (source[position] == '+' || source[position] == '-') {
                ++position;
                ++column;
            }

            while (position < source.size() && std::isdigit(static_cast<unsigned char>(source[position]))) {
                ++position;
                ++column;
            }

            if (position < source.size() && source[position] == '.') {
                ++position;
                ++column;
                while (position < source.size() && std::isdigit(static_cast<unsigned char>(source[position]))) {
                    ++position;
                    ++column;
                }
            }

            if (position < source.size() && (source[position] == 'e' || source[position] == 'E')) {
                const std::size_t exponent_start = position;
                const std::size_t exponent_column = column;
                std::size_t exponent_position = position + 1;
                std::size_t exponent_digits = 0;

                if (exponent_position < source.size() &&
                    (source[exponent_position] == '+' || source[exponent_position] == '-')) {
                    ++exponent_position;
                }

                while (exponent_position < source.size() &&
                       std::isdigit(static_cast<unsigned char>(source[exponent_position]))) {
                    ++exponent_position;
                    ++exponent_digits;
                }

                if (exponent_digits > 0) {
                    column += exponent_position - position;
                    position = exponent_position;
                } else {
                    (void)exponent_start;
                    (void)exponent_column;
                }
            }

            add_token(TokenType::Number, start, start_line, start_column);
            continue;
        }

        if (starts_identifier(source, position)) {
            const std::size_t start = position;
            const std::size_t start_line = line;
            const std::size_t start_column = column;

            if (source[position] == '-') {
                ++position;
                ++column;
            }

            while (position < source.size()) {
                if (is_name_codepoint(source, position)) {
                    if (static_cast<unsigned char>(source[position]) < 0x80) {
                        ++position;
                        ++column;
                    } else {
                        std::uint32_t codepoint = 0;
                        std::size_t length = 0;
                        if (!decode_utf8(source, position, codepoint, length)) {
                            throw std::runtime_error(
                                filepath.string() + ":" + std::to_string(line) + ":" +
                                std::to_string(column) + ": Invalid UTF-8 sequence");
                        }
                        position += length;
                        column += length;
                    }
                    continue;
                }

                if (source[position] == '\\') {
                    consume_escape(source, position, column);
                    continue;
                }

                break;
            }

            add_token(TokenType::Identifier, start, start_line, start_column);
            continue;
        }

        if (current == '#' && position + 1 < source.size() &&
            (is_name_codepoint(source, position + 1) || is_valid_escape(source, position + 1))) {
            const std::size_t start = position;
            const std::size_t start_line = line;
            const std::size_t start_column = column;
            ++position;
            ++column;

            while (position < source.size()) {
                if (is_name_codepoint(source, position)) {
                    if (static_cast<unsigned char>(source[position]) < 0x80) {
                        ++position;
                        ++column;
                    } else {
                        std::uint32_t codepoint = 0;
                        std::size_t length = 0;
                        if (!decode_utf8(source, position, codepoint, length)) {
                            throw std::runtime_error("Invalid UTF-8 sequence in hash");
                        }
                        position += length;
                        column += length;
                    }
                    continue;
                }
                if (source[position] == '\\') {
                    consume_escape(source, position, column);
                    continue;
                }
                break;
            }

            add_token(TokenType::Hash, start, start_line, start_column);
            continue;
        }

        if (current == '@' && position + 1 < source.size() && starts_identifier(source, position + 1)) {
            const std::size_t start = position;
            const std::size_t start_line = line;
            const std::size_t start_column = column;
            ++position;
            ++column;

            if (source[position] == '-') {
                ++position;
                ++column;
            }

            while (position < source.size()) {
                if (is_name_codepoint(source, position)) {
                    if (static_cast<unsigned char>(source[position]) < 0x80) {
                        ++position;
                        ++column;
                    } else {
                        std::uint32_t codepoint = 0;
                        std::size_t length = 0;
                        if (!decode_utf8(source, position, codepoint, length)) {
                            throw std::runtime_error("Invalid UTF-8 sequence in at-keyword");
                        }
                        position += length;
                        column += length;
                    }
                    continue;
                }
                if (source[position] == '\\') {
                    consume_escape(source, position, column);
                    continue;
                }
                break;
            }

            add_token(TokenType::AtKeyword, start, start_line, start_column);
            continue;
        }

        if (current == '"' || current == '\'') {
            const std::size_t start = position;
            const std::size_t start_line = line;
            const std::size_t start_column = column;
            const char quote = current;
            ++position;
            ++column;
            bool closed = false;

            while (position < source.size()) {
                if (source[position] == quote) {
                    ++position;
                    ++column;
                    closed = true;
                    break;
                }

                if (source[position] == '\\') {
                    if (!is_valid_escape(source, position)) {
                        throw std::runtime_error(
                            filepath.string() + ":" + std::to_string(line) + ":" +
                            std::to_string(column) + ": Invalid string escape");
                    }
                    consume_escape(source, position, column);
                    continue;
                }

                if (source[position] == '\n' || source[position] == '\r') {
                    throw std::runtime_error(
                        filepath.string() + ":" + std::to_string(line) + ":" +
                        std::to_string(column) + ": Unescaped newline in string");
                }

                if (static_cast<unsigned char>(source[position]) >= 0x80) {
                    std::uint32_t codepoint = 0;
                    std::size_t length = 0;
                    if (!decode_utf8(source, position, codepoint, length)) {
                        throw std::runtime_error("Invalid UTF-8 sequence in string");
                    }
                    position += length;
                    column += length;
                } else {
                    ++position;
                    ++column;
                }
            }

            if (!closed) {
                throw std::runtime_error(
                    filepath.string() + ":" + std::to_string(start_line) + ":" +
                    std::to_string(start_column) + ": Unterminated string");
            }

            add_token(TokenType::String, start, start_line, start_column);
            continue;
        }

        TokenType type;
        std::size_t width = 1;

        if (current == '~' && position + 1 < source.size() && source[position + 1] == '=') {
            type = TokenType::IncludesMatch;
            width = 2;
        } else if (current == '|' && position + 1 < source.size() && source[position + 1] == '=') {
            type = TokenType::DashMatch;
            width = 2;
        } else if (current == '^' && position + 1 < source.size() && source[position + 1] == '=') {
            type = TokenType::PrefixMatch;
            width = 2;
        } else if (current == '$' && position + 1 < source.size() && source[position + 1] == '=') {
            type = TokenType::SuffixMatch;
            width = 2;
        } else if (current == '*' && position + 1 < source.size() && source[position + 1] == '=') {
            type = TokenType::SubstringMatch;
            width = 2;
        } else if (current == '|' && position + 1 < source.size() && source[position + 1] == '|') {
            type = TokenType::Column;
            width = 2;
        } else {
            switch (current) {
                case ':': type = TokenType::Colon; break;
                case ';': type = TokenType::Semicolon; break;
                case '{': type = TokenType::LeftBrace; break;
                case '}': type = TokenType::RightBrace; break;
                case '<': type = TokenType::LessThan; break;
                case '>': type = TokenType::GreaterThan; break;
                case '[': type = TokenType::LeftBracket; break;
                case ']': type = TokenType::RightBracket; break;
                case '(': type = TokenType::LeftParen; break;
                case ')': type = TokenType::RightParen; break;
                case '=': type = TokenType::Equals; break;
                case ',': type = TokenType::Comma; break;
                case '%': type = TokenType::Percent; break;
                case '+': type = TokenType::Plus; break;
                case '-': type = TokenType::Minus; break;
                case '*': type = TokenType::Asterisk; break;
                case '/': type = TokenType::Slash; break;
                case '~': type = TokenType::Tilde; break;
                case '|': type = TokenType::Pipe; break;
                case '^': type = TokenType::Caret; break;
                case '$': type = TokenType::Dollar; break;
                case '&': type = TokenType::Ampersand; break;
                case '!': type = TokenType::Bang; break;
                case '?': type = TokenType::QuestionMark; break;
                case '.': type = TokenType::Dot; break;
                case '\\': type = TokenType::Backslash; break;
                default:
                    throw std::runtime_error(
                        filepath.string() + ":" + std::to_string(line) + ":" +
                        std::to_string(column) + ": Unexpected character");
            }
        }

        const std::size_t start = position;
        const std::size_t start_line = line;
        const std::size_t start_column = column;
        position += width;
        column += width;
        tokens.push_back({type, source.substr(start, width), {filepath, start_line, start_column}});
    }

    tokens.push_back({TokenType::EndOfFile, "", {filepath, line, column}});

    if (csam_debug) {
        std::cout << "Lexer: Finished tokenizing " << filepath.string() << '\n';
    }

    return tokens;
}
