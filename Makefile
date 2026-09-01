CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -pedantic -Icompiler/include

TARGET_BASE = csam

# Windows builds use the native executable suffix. Keep the compiler
# invocation identical so POSIX, MSYS2, and Cygwin builds remain simple.
ifeq ($(OS),Windows_NT)
    EXE_SUFFIX = .exe
else
    EXE_SUFFIX =
endif

TARGET = $(TARGET_BASE)$(EXE_SUFFIX)

SRC = $(wildcard compiler/src/*.cpp)
TEST_COMMON_SRC = compiler/src/lexer.cpp \
                  compiler/src/parser.cpp \
                  compiler/src/ast.cpp \
                  compiler/src/token.cpp \
                  compiler/src/debug.cpp

LEXER_TEST = tests/test_lexer$(EXE_SUFFIX)
PARSER_TEST = tests/test_parser$(EXE_SUFFIX)

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

$(LEXER_TEST): tests/test_lexer.cpp $(TEST_COMMON_SRC)
	$(CXX) $(CXXFLAGS) tests/test_lexer.cpp $(TEST_COMMON_SRC) -o $(LEXER_TEST)

$(PARSER_TEST): tests/test_parser.cpp $(TEST_COMMON_SRC)
	$(CXX) $(CXXFLAGS) tests/test_parser.cpp $(TEST_COMMON_SRC) -o $(PARSER_TEST)

test: $(LEXER_TEST) $(PARSER_TEST)
	./$(LEXER_TEST)
	./$(PARSER_TEST)

clean:
ifeq ($(OS),Windows_NT)
	ifeq ($(findstring sh,$(notdir $(SHELL))),sh)
		rm -f $(TARGET) $(LEXER_TEST) $(PARSER_TEST)
	else
		del /Q /F $(TARGET) $(LEXER_TEST) $(PARSER_TEST)
	endif
else
	rm -f $(TARGET) $(LEXER_TEST) $(PARSER_TEST)
endif
