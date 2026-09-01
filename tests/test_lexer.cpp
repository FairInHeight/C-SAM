#include "lexer.hpp"
#include "token.hpp"

#include <cassert>
#include <string>
#include <vector>

static std::vector<Token> lex(const std::string& source)
{
    Lexer lexer(source, "test.csam");
    return lexer.tokenize();
}

int main()
{
    {
        const auto tokens = lex("div { color: red; }");
        assert(tokens.size() == 8);
        assert(tokens[0].type == TokenType::Identifier);
        assert(tokens[0].value == "div");
        assert(tokens[1].type == TokenType::LeftBrace);
        assert(tokens[2].type == TokenType::Identifier);
        assert(tokens[2].value == "color");
        assert(tokens[3].type == TokenType::Colon);
        assert(tokens[4].type == TokenType::Identifier);
        assert(tokens[4].value == "red");
        assert(tokens[5].type == TokenType::Semicolon);
        assert(tokens[6].type == TokenType::RightBrace);
        assert(tokens[7].type == TokenType::EndOfFile);
    }

    {
        const auto tokens = lex("// comment\ndiv /* comment */ { content: \"hello\"; }");
        assert(tokens[0].value == "div");
        assert(tokens[1].type == TokenType::LeftBrace);
        assert(tokens[2].value == "content");
        assert(tokens[4].type == TokenType::String);
        assert(tokens[4].value == "\"hello\"");
        assert(tokens.back().type == TokenType::EndOfFile);
    }

    {
        const auto tokens = lex("#id [x] (y) <z> = a, 42.5");
        assert(tokens[0].type == TokenType::Hash);
        assert(tokens[1].type == TokenType::LeftBracket);
        assert(tokens[2].type == TokenType::Identifier);
        assert(tokens[3].type == TokenType::RightBracket);
        assert(tokens[4].type == TokenType::LeftParen);
        assert(tokens[5].type == TokenType::Identifier);
        assert(tokens[6].type == TokenType::RightParen);
        assert(tokens[7].type == TokenType::LeftAngle);
        assert(tokens[8].type == TokenType::Identifier);
        assert(tokens[9].type == TokenType::RightAngle);
        assert(tokens[10].type == TokenType::Equals);
        assert(tokens[11].type == TokenType::Identifier);
        assert(tokens[12].type == TokenType::Comma);
        assert(tokens[13].type == TokenType::Number);
        assert(tokens[13].value == "42.5");
        assert(tokens[14].type == TokenType::EndOfFile);
    }

    {
        const auto tokens = lex("first\nsecond");
        assert(tokens[0].location.line == 1);
        assert(tokens[0].location.column == 1);
        assert(tokens[1].location.line == 2);
        assert(tokens[1].location.column == 1);
        assert(tokens.back().type == TokenType::EndOfFile);
    }

    return 0;
}
