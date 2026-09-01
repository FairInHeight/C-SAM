# C SAM Tests

The test suite is intentionally lightweight and has no external dependencies.

- `test_lexer.cpp` exercises lexical tokenization and source locations.
- `test_parser.cpp` exercises parser structure and basic AST construction through the parser.
- `test_ast.cpp` exercises AST node construction, relationships, stored values, and child management directly.

Run the complete suite from the project root with:

```sh
make test
```

The test executables are built in `tests/` and can also be run individually when debugging a specific compiler layer.
