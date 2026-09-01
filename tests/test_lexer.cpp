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

    {
        const auto tokens = lex("10 10.5 .5 5. -10 +10 1e3 -1.5e-2 10px 50%");
        assert(tokens[0].type == TokenType::Number && tokens[0].value == "10");
        assert(tokens[1].type == TokenType::Number && tokens[1].value == "10.5");
        assert(tokens[2].type == TokenType::Number && tokens[2].value == ".5");
        assert(tokens[3].type == TokenType::Number && tokens[3].value == "5.");
        assert(tokens[4].type == TokenType::Number && tokens[4].value == "-10");
        assert(tokens[5].type == TokenType::Number && tokens[5].value == "+10");
        assert(tokens[6].type == TokenType::Number && tokens[6].value == "1e3");
        assert(tokens[7].type == TokenType::Number && tokens[7].value == "-1.5e-2");
        assert(tokens[8].type == TokenType::Number && tokens[8].value == "10");
        assert(tokens[9].type == TokenType::Identifier && tokens[9].value == "px");
        assert(tokens[10].type == TokenType::Number && tokens[10].value == "50");
        assert(tokens[11].type == TokenType::Percent && tokens[11].value == "%");
        assert(tokens[12].type == TokenType::EndOfFile);
    }

    {
        const auto tokens = lex("foo --primary-color café --тема");
        assert(tokens[0].type == TokenType::Identifier);
        assert(tokens[0].value == "foo");
        assert(tokens[1].type == TokenType::Identifier);
        assert(tokens[1].value == "--primary-color");
        assert(tokens[2].type == TokenType::Identifier);
        assert(tokens[2].value == "café");
        assert(tokens[3].type == TokenType::Identifier);
        assert(tokens[3].value == "--тема");
        assert(tokens[4].type == TokenType::EndOfFile);
    }

    {
        const auto tokens = lex("'single' \"double\" \"escaped \\\" quote\"");
        assert(tokens[0].type == TokenType::String);
        assert(tokens[0].value == "'single'");
        assert(tokens[1].type == TokenType::String);
        assert(tokens[1].value == "\"double\"");
        assert(tokens[2].type == TokenType::String);
        assert(tokens[2].value == "\"escaped \\\" quote\"");
        assert(tokens[3].type == TokenType::EndOfFile);
    }

    {
        const auto tokens = lex("@media #foo\\+bar ~= |= ^= $= *= || & ! ? .foo .5");
        assert(tokens[0].type == TokenType::AtKeyword);
        assert(tokens[0].value == "@media");
        assert(tokens[1].type == TokenType::Hash);
        assert(tokens[1].value == "#foo\\+bar");
        assert(tokens[2].type == TokenType::IncludesMatch);
        assert(tokens[3].type == TokenType::DashMatch);
        assert(tokens[4].type == TokenType::PrefixMatch);
        assert(tokens[5].type == TokenType::SuffixMatch);
        assert(tokens[6].type == TokenType::SubstringMatch);
        assert(tokens[7].type == TokenType::Column);
        assert(tokens[8].type == TokenType::Ampersand);
        assert(tokens[9].type == TokenType::Bang);
        assert(tokens[10].type == TokenType::QuestionMark);
        assert(tokens[11].type == TokenType::Dot);
        assert(tokens[12].type == TokenType::Identifier);
        assert(tokens[13].type == TokenType::Number);
        assert(tokens[13].value == ".5");
        assert(tokens[14].type == TokenType::EndOfFile);
    }

    {
        const auto tokens = lex("+ - * / > < ~ | ^ $ \\ ");
        assert(tokens[0].type == TokenType::Plus);
        assert(tokens[1].type == TokenType::Minus);
        assert(tokens[2].type == TokenType::Asterisk);
        assert(tokens[3].type == TokenType::Slash);
        assert(tokens[4].type == TokenType::GreaterThan);
        assert(tokens[5].type == TokenType::LessThan);
        assert(tokens[6].type == TokenType::Tilde);
        assert(tokens[7].type == TokenType::Pipe);
        assert(tokens[8].type == TokenType::Caret);
        assert(tokens[9].type == TokenType::Dollar);
        assert(tokens[10].type == TokenType::Backslash);
        assert(tokens[11].type == TokenType::EndOfFile);
    }

    return 0;
}
