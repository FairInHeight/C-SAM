# C SAM Tests

The test suite is intentionally lightweight and has no external dependencies.

- `test_lexer.cpp` exercises lexical tokenization and source locations.
- `test_parser.cpp` exercises basic AST construction and parser structure.

Tests are currently source-level fixtures. Build integration will be added through the project's build system once the initial coverage is established.
