CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -pedantic -Icompiler/include

TARGET = csam

SRC = $(wildcard compiler/src/*.cpp)
TEST_COMMON_SRC = compiler/src/lexer.cpp \
                  compiler/src/parser.cpp \
                  compiler/src/ast.cpp \
                  compiler/src/debug.cpp \
                  compiler/src/value.cpp

LEXER_TEST = tests/test_lexer
PARSER_TEST = tests/test_parser
AST_TEST = tests/test_ast

.PHONY: all test clean clean-test auto

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

$(LEXER_TEST): tests/test_lexer.cpp $(TEST_COMMON_SRC)
	$(CXX) $(CXXFLAGS) tests/test_lexer.cpp $(TEST_COMMON_SRC) -o $(LEXER_TEST)

$(PARSER_TEST): tests/test_parser.cpp $(TEST_COMMON_SRC)
	$(CXX) $(CXXFLAGS) tests/test_parser.cpp $(TEST_COMMON_SRC) -o $(PARSER_TEST)

$(AST_TEST): tests/test_ast.cpp $(TEST_COMMON_SRC)
	$(CXX) $(CXXFLAGS) tests/test_ast.cpp $(TEST_COMMON_SRC) -o $(AST_TEST)

test: $(LEXER_TEST) $(PARSER_TEST) $(AST_TEST)
	@echo "Running lexer tests..."
	@./$(LEXER_TEST)
	@echo "Lexer tests: PASS"
	@echo "Running parser tests..."
	@./$(PARSER_TEST)
	@echo "Parser tests: PASS"
	@echo "Running AST tests..."
	@./$(AST_TEST)
	@echo "AST tests: PASS"
	@echo "All tests passed."

clean-test:
	rm -f $(LEXER_TEST) $(PARSER_TEST) $(AST_TEST)

clean:
	rm -f $(TARGET) $(LEXER_TEST) $(PARSER_TEST) $(AST_TEST)

auto:
	$(MAKE) clean
	$(MAKE)
	./$(TARGET) -d test.csam
	$(MAKE) test
