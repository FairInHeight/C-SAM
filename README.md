# C SAM

C SAM is a C++23 compiler project for a new web language that combines HTML-style document structure and CSS styling into a single source file.

The project is being built incrementally as a compiler frontend first, then a semantic analyzer, and finally a code generator. The current frontend can tokenize C SAM, validate its grammar, construct an ordered AST, and convert a growing subset of CSS-style values into semantic value nodes.

The language is still being designed. `test.csam` is the current de facto grammar and integration reference.

## Project goals

C SAM is designed to:

- Combine HTML and CSS into one source file per page.
- Be immediately understandable to people familiar with web development.
- Provide a natural bridge from the C-family of languages into web development.
- Keep syntax small, predictable, and easy to parse.
- Preserve source locations throughout the compiler for clear diagnostics.
- Keep compiler filesystem handling platform-independent with `std::filesystem`.
- Eventually compile one C SAM source into generated HTML and CSS.

## Current status

The frontend is currently **stable and regression-tested**.

Implemented:

- C++23 compiler build.
- Command-line argument parsing with `-d` debug mode.
- Source-file loading with standard C++ filesystem types.
- Lexer with source locations.
- C-style line and block comments.
- Strings and escape handling.
- Signed numbers, decimals, percentages, and exponent notation.
- CSS-oriented identifiers, hashes, at-keywords, punctuation, and selector operators.
- Unicode-aware identifier handling and UTF-8 validation.
- Transactional handling of incomplete numeric exponents.
- Recursive-descent parser.
- Required `:root` document structure.
- Nested tags and tag content.
- CSS-style properties.
- C-like `var name = value;` declarations.
- Parser-side delimiter validation.
- Source-aware parser diagnostics.
- Semantic number, dimension, percentage, and string value nodes.
- `RawValueNode` for syntactically valid but not-yet-modeled values.
- Function value nodes with comma-separated and nested arguments.
- Ordered AST child storage.
- Explicit AST ownership through `std::unique_ptr`.
- Dedicated lexer, parser, AST, and value-parser regression suites.
- Makefile build, test, cleanup, and automated integration targets.

Not implemented yet:

- Semantic variable resolution.
- Semantic scope validation.
- Duplicate-definition checks.
- Value/type compatibility checking.
- CSS property and unit validation.
- Full CSS selector semantics.
- HTML semantic validation.
- Semantic-analysis passes.
- HTML/CSS code generation.

## Compiler pipeline

```text
C SAM source
     |
     v
   Lexer
     |
     | Token stream + SourceLocation
     v
   Parser
     |
     | Grammar + nesting + value parsing
     v
    AST
     |
     | NEXT
     v
Semantic analysis
     |
     | FUTURE
     v
Generated HTML + CSS
```

The lexer remains deliberately lexical. The parser owns grammar and the first semantic interpretation of values. Semantic analysis will become responsible for meaning, resolution, validation, and consistency.

## Source layout

```text
.
├── compiler
│   ├── include
│   │   ├── args.hpp
│   │   ├── ast.hpp
│   │   ├── debug.hpp
│   │   ├── lexer.hpp
│   │   ├── parser.hpp
│   │   ├── token.hpp
│   │   ├── value.hpp
│   │   └── value_parser.hpp
│   └── src
│       ├── args.cpp
│       ├── ast.cpp
│       ├── debug.cpp
│       ├── lexer.cpp
│       ├── main.cpp
│       ├── parser.cpp
│       ├── value.cpp
│       └── value_parser.cpp
├── tests
│   ├── test_ast.cpp
│   ├── test_lexer.cpp
│   ├── test_parser.cpp
│   ├── test_value_parser.cpp
│   └── README.md
├── test.csam
├── test.css
├── test.html
├── bad.csam
├── Makefile
└── README.md
```

## C SAM syntax

### Root

Every C SAM file must begin with exactly one `:root` block:

```csam
:root
{
}
```

There is no colon syntax on ordinary tags. `:root` is special and is the required document root.

### Tags

A tag is an identifier followed by either `{` or `<`.

```csam
header
{
}

h1 <"Welcome to my website">

h1 <"Welcome">
{
    font-size: 36px;
}
```

Tags may be nested to arbitrary depth.

### Tag content

Angle brackets `< >` delimit tag content. Content is currently retained as tokens in a `ContentNode`; strings are the primary content form.

```csam
p
<
    "This is my first C-SAM website.\n"
    "It is beautiful, elegant, and predictable.\n"
>
```

Using `< >` distinguishes tag content from CSS-style declarations and leaves room for richer content in future versions.

### Properties

CSS-style properties use a colon and semicolon:

```csam
body
{
    font-family: Arial, sans-serif;
    margin: 0;
    background: #f4f4f4;
}
```

Property values are passed through the semantic value parser.

```csam
margin: 40px auto;
opacity: 0.5;
width: 50%;
background: mycolor;
```

The parser currently describes the shape of these values. It does not yet decide whether a particular CSS property, unit, identifier, or function is semantically valid.

### Variables

Variables use a C-like declaration form:

```csam
var mycolor = #8800ff;
```

Variable values use the same semantic value parsing machinery as properties. Variable resolution, duplicate-definition checks, type compatibility, and semantic scope rules are deferred to semantic analysis.

## Semantic values

The lexer remains granular while the value parser interprets token sequences into semantic nodes.

```text
10
```

becomes a `NumberValueNode`, while:

```text
10px
```

is lexed as:

```text
Number("10")
Identifier("px")
```

and becomes a `DimensionValueNode` because the number and unit are directly adjacent in the source.

The current value hierarchy is:

```text
ValueNode
├── NumberValueNode
├── DimensionValueNode
├── PercentageValueNode
├── StringValueNode
├── FunctionValueNode
└── RawValueNode
```

The intended progression is:

```text
characters → tokens → semantic values → AST
```

Numeric text is preserved as its source representation instead of immediately converting it to floating point. Numeric validation and normalization can therefore be performed later without losing the original representation.

### Dimensions and adjacency

```csam
width: 10px;
```

is one dimension value.

```csam
width: 10 px;
```

is a number followed by a raw value rather than being silently combined.

### Percentages

```csam
width: 50%;
margin: -25%;
```

produce percentage value nodes when the number and percent sign are directly adjacent.

### Raw values

`RawValueNode` is an intentional lossless fallback for value syntax that the compiler recognizes lexically but has not yet given a dedicated semantic type.

Values such as `auto`, `white`, and `mycolor` can currently remain raw. A raw value is **not automatically a parser error**; semantic analysis will determine whether it is a literal, reference, keyword, identifier, or invalid value in its context.

## Functions and nested values

Function calls are represented by `FunctionValueNode`.

```csam
color: rgb(255, 0, 0);
width: calc(100% - 20px);
transform: scale(2, translate(10px, 20px));
```

Arguments are stored as separate collections, so comma-separated arguments remain distinct. Each argument can contain multiple semantic values and nested functions.

Conceptually:

```text
Function: scale
├── Argument
│   └── Number: 2
└── Argument
    └── Function: translate
        ├── Argument
        │   └── Dimension: 10px
        └── Argument
            └── Dimension: 20px
```

Function recognition depends on source adjacency. `calc(100%)` is function syntax; an identifier separated from `(` by whitespace is not automatically interpreted as a function call.

Malformed function arguments are rejected, including empty arguments, trailing commas, and missing closing delimiters when the parser reaches the invalid state.

CSS-specific functions such as `calc()`, `var()`, color functions, URLs, and gradients are not individually validated yet.

## Basic grammar

```text
file
    -> :root block EOF

block
    -> { declaration* }

declaration
    -> variable
    -> property
    -> tag

variable
    -> "var" IDENTIFIER = value ;
    -> IDENTIFIER = value ;

property
    -> IDENTIFIER : value ;

tag
    -> IDENTIFIER block
    -> IDENTIFIER content
    -> IDENTIFIER content block

content
    -> < content_tokens >

value
    -> semantic_value+
```

A tag name must be followed by `{` or `<`. A property name must be followed by `:`. A variable must use a valid assignment form.

## Lexing

The lexer currently recognizes:

- Identifiers, including CSS-style names.
- Unicode identifiers.
- Strings and escapes.
- Numbers, signs, decimals, and exponent forms.
- Percent tokens.
- Hash values.
- At-keywords.
- `:`, `;`, `,`, and `=`.
- `{}`, `[]`, and `()`.
- `+`, `-`, `*`, `/`, `<`, `>`, `~`, `|`, `^`, `$`, `&`, `!`, `?`, `.`, and `\\`.
- `~=`, `|=`, `^=`, `$=`, `*=`, and `||` selector-oriented operators.
- End-of-file.
- `//` line comments.
- `/* ... */` block comments.

Incomplete exponent parsing is transactional. For example, `1e-` does not consume `e-` as part of the number, leaving those characters available for subsequent tokenization.

UTF-8 is validated while processing Unicode identifiers and strings. Invalid sequences are rejected instead of silently producing invalid token text.

Strings reject invalid escapes, unescaped newlines, and unterminated input. Unterminated block comments are also rejected with source-aware errors.

### Angle brackets

`<` and `>` have one lexical identity: `LessThan` and `GreaterThan`. Their meaning is contextual. Tag content uses them as delimiters, while future selector grammar may use them for CSS relationships or punctuation.

### Whitespace

Whitespace is not emitted as a token. When semantic interpretation depends on adjacency, the parser compares source locations.

This is used for:

- `10px` versus `10 px`.
- `50%` versus separated number/percent syntax.
- `calc(` versus an identifier separated from `(`.

## Tokens and source locations

Each token contains:

```text
TokenType type
std::string value
SourceLocation location
```

`SourceLocation` contains:

```text
filepath
line
column
```

The filepath uses `std::filesystem::path`, keeping compiler source handling independent of Unix `/` versus Windows `\\` path syntax.

The token vocabulary is centralized in `TokenType`, while `token_type_name()` provides readable names for debug output.

## Parser

The parser uses recursive-descent parsing and is responsible for document grammar, nesting, delimiter validation, and the first semantic interpretation of values.

It currently:

- Requires `:root` at the beginning of every file.
- Parses variables, properties, tags, and tag content.
- Supports arbitrary tag nesting.
- Maintains a non-owning AST scope stack.
- Validates `{}`, `[]`, and `()` delimiter pairing.
- Handles `<` and `>` according to grammar context.
- Parses numbers, dimensions, percentages, strings, raw values, and functions.
- Supports nested function arguments.
- Preserves unsupported value syntax through raw value nodes.
- Reports source-aware syntax errors.
- Builds the AST while parsing.

Delimiter validation and scope tracking are separate concerns:

```text
Delimiter stack
    -> Are punctuation delimiters properly paired?

AST scope stack
    -> Which document node is currently receiving children?
```

The scope stack contains non-owning pointers; ownership remains with the AST.

## AST

```text
ASTNode
├── RootNode
├── TagNode
├── ContentNode
├── PropertyNode
└── VariableNode

ValueNode
├── NumberValueNode
├── DimensionValueNode
├── PercentageValueNode
├── StringValueNode
├── FunctionValueNode
└── RawValueNode
```

Every AST node stores its source location.

### Ownership

AST ownership uses `std::unique_ptr`. The AST owns its nodes; the parser keeps only non-owning pointers to active scope nodes.

### Ordered children

`RootNode` and `TagNode` maintain one ordered child collection of `ASTNode` objects. This preserves the original sibling order between variables, properties, content, and nested tags.

`TagNode` also keeps a non-owning pointer to its `ContentNode` for convenient access while the content node remains owned by the ordered child collection.

### Properties and variables

Properties and variables store semantic `ValueNode` objects rather than only raw token vectors.

For example:

```csam
padding: 30px;
margin: 40px auto;
```

becomes conceptually:

```text
Property: padding
    Dimension: 30px

Property: margin
    Dimension: 40px
    Raw: auto
```

This lets later compiler stages operate on structured meaning rather than reconstructing source tokens.

## Diagnostics and error handling

Parser errors use source locations whenever possible:

```text
Parser: Expected ':' after property name at test.csam:1:15
```

`consume()` and `unexpected()` provide centralized parser error construction.

Negative tests verify both failure and diagnostic content for important malformed states, including invalid roots, malformed properties, malformed variables, missing values, missing semicolons, unmatched delimiters, malformed function arguments, and unexpected trailing input.

The diagnostic system is intentionally lightweight for now. A structured diagnostic abstraction can be introduced when semantic analysis establishes its requirements.

## Debug mode

Debug output is controlled by the global `csam_debug` flag and enabled with `-d`.

Normal successful compilation is intentionally quiet:

```text
./csam test.csam
```

Debug mode exposes the frontend state:

```text
./csam -d test.csam
```

Current debug output includes:

- Lexer start/finish messages.
- Token types, values, and source locations.
- The constructed AST.
- Semantic value node kinds such as `Number`, `Dimension`, `Percentage`, `Function`, and `Raw`.

## Command-line arguments

The compiler accepts one or more source paths with an optional flags argument.

```text
./csam test.csam
./csam -d test.csam
./csam test.csam other.csam
./csam -d test.csam other.csam
```

Currently supported flag:

```text
-d    Enable compiler debug output
```

Invalid arguments and invalid flags return non-zero exit codes.

## Building and testing

The Makefile provides explicit compiler, test, cleanup, and automation targets.

### Build

```text
make
```

Builds `csam` from all compiler source files.

### Test

```text
make test
```

Builds and runs all four regression suites:

```text
Lexer tests
Parser tests
AST tests
Value parser tests
```

A successful run ends with:

```text
All tests passed.
```

### Clean

```text
make clean
```

Removes the compiler executable and all test executables.

### Clean tests only

```text
make clean-test
```

Removes only the four test executables. It does not remove the compiler executable, rebuild tests, or run tests.

### Automatic development cycle

```text
make auto
```

The target performs:

```text
make clean
make
./csam -d test.csam
make test
```

Each stage is chained with `&&`, so a non-zero result stops the pipeline.

The complete `make auto` pipeline has been successfully exercised after the latest frontend stabilization work.

## Test coverage

### Lexer tests

Lexer coverage includes punctuation, signed and decimal numbers, exponent forms, incomplete exponents, percentages, strings, escapes, hashes, identifiers, Unicode identifiers, comments, selector-oriented operators, and source locations.

### Parser tests

Parser coverage includes the required root, tags, nesting, content, properties, variables, ordered AST children, semantic values, functions, nested functions, malformed declarations, delimiter errors, malformed function arguments, missing values, and unexpected trailing input.

### AST tests

AST tests verify construction, node types, accessors, ownership-oriented behavior, child ordering, and semantic value nodes.

### Value parser tests

Value-parser coverage includes:

- Numbers.
- Dimensions.
- Percentages.
- Strings.
- Raw values.
- Negative values.
- Exponent notation.
- Whitespace boundaries.
- Functions.
- Empty functions.
- Comma-separated arguments.
- Nested functions.
- `calc()`-style expressions.
- Empty arguments.
- Trailing commas.
- Missing closing delimiters.
- Empty values.

The tests intentionally cover both positive and negative paths so semantic analysis can be built against a stable syntactic baseline.

## Integration fixture

`test.csam` is the current valid end-to-end frontend fixture. It demonstrates variables, nested tags, tag content, CSS-style properties, colors, identifiers, dimensions, multiple property values, and escaped strings.

The repository also contains `test.html` and `test.css` reference outputs/examples corresponding to the current fixture, providing a useful target for future code generation.

`bad.csam` remains a deliberately malformed fixture for parser/error testing.

## Recent frontend progress

The frontend has undergone a substantial stabilization pass.

### Lexer

The lexer was expanded with CSS-oriented lexical support, Unicode identifiers, UTF-8 validation, escapes, comments, source locations, signed and exponent-form numbers, hashes, at-keywords, and multi-character selector operators.

Incomplete exponents are handled transactionally so malformed numeric suffixes do not consume tokens that belong to following syntax.

### Parser and value parser

Value parsing was separated into a dedicated `ValueParser`. The structural `Parser` now delegates value interpretation while retaining responsibility for document grammar and scope management.

The value parser constructs semantic nodes for numbers, dimensions, percentages, strings, functions, and raw/unresolved values. Function arguments are recursively represented, allowing nested expressions to survive parsing without flattening their structure.

### AST

Properties and variables were moved toward semantic `ValueNode` storage. The AST was tightened around explicit ownership and ordered child storage, with parser scope tracking remaining non-owning.

### Diagnostics

Parser diagnostics now consistently include filepath, line, and column information. Negative tests were strengthened to check the actual diagnostic message instead of only checking whether parsing failed.

### Build/test toolkit

The Makefile was brought into sync with the value parser and its fourth test suite. It now provides `make`, `make test`, `make clean`, `make clean-test`, and `make auto`.

`make auto` stops on the first failure. The full pipeline has been verified successfully through a clean build, debug compilation/run of `test.csam`, and all four regression suites.

## Current design principles

1. **One source file per page.** HTML-style structure and CSS-style declarations live together.
2. **CSS familiarity.** Styling keeps familiar CSS declarations and punctuation.
3. **C-family familiarity.** Variables use a simple C-like declaration syntax and comments follow C conventions.
4. **Tags are raw identifiers.** Ordinary tags do not use a colon.
5. **`{}` defines structural blocks.** Blocks contain declarations and nested tags.
6. **`<>` defines tag content.** Content is distinct from CSS declarations and remains extensible.
7. **`:` belongs to declarations.** It separates a property name from its value.
8. **`;` terminates declarations.** Properties and variables require it.
9. **The lexer stays lexical.** It recognizes tokens and lexical validity without deciding grammar or semantic meaning.
10. **The parser owns grammar.** It validates structure, nesting, delimiters, and declaration forms.
11. **The value parser owns value interpretation.** Token sequences become semantic value nodes where the language currently defines their meaning.
12. **Angle brackets are contextual.** `<` and `>` have one lexical identity and receive meaning from grammar context.
13. **Semantic values belong in the AST.** Meaningful values are represented independently from raw tokens.
14. **Raw fallbacks are intentional.** Unsupported CSS constructs remain representable rather than being discarded.
15. **Raw values are unresolved, not necessarily invalid.** Semantic analysis will determine their meaning and validity.
16. **The AST preserves source order.** Sibling nodes remain ordered as written.
17. **Ownership stays explicit.** AST ownership uses `std::unique_ptr`; parser scope tracking is non-owning.
18. **Source locations remain attached to syntax.** Later stages can report errors against original source positions.
19. **Tests protect compiler stages independently.** Lexer, parser, AST, and value-parser behavior are tested separately.
20. **Do not prematurely over-model CSS.** The frontend should preserve useful syntax until semantic analysis determines what requires dedicated representation.

## Next roadmap

The immediate frontend stabilization phase is complete.

### Completed

```text
[✓] Build/test infrastructure audit
[✓] AST ↔ value-parser ownership/interface audit
[✓] Negative/error-path coverage
[✓] Full clean build + debug integration + regression tests
```

### Next: semantic analysis

The next major phase will build on the current AST rather than replacing the parser.

Initial semantic-analysis goals:

- Establish lexical/document scopes for variables.
- Detect duplicate declarations where prohibited.
- Resolve variable references.
- Distinguish literals, keywords, identifiers, and references where appropriate.
- Validate semantic value relationships.
- Establish the initial C SAM semantic/type model.
- Produce clearer semantic diagnostics.

### Later: code generation

Once semantic analysis can guarantee a valid program representation, the compiler can begin generating HTML and CSS.

```text
Source
  ↓
Lexer
  ↓
Parser
  ↓
AST
  ↓
Semantic analysis
  ↓
Validated semantic representation
  ↓
HTML/CSS generation
```

## Refactoring notes

Large architectural changes are intentionally being deferred until semantic analysis tells us what abstractions are actually required.

One cleanup candidate worth evaluating is large token-dispatch logic. When a switch only maps enum values to fixed data, a compile-time lookup table or array may be cleaner than a large switch. This should be evaluated case-by-case rather than replacing switches indiscriminately.

A structured diagnostic object may eventually replace plain exception strings. That refactor is intentionally deferred until semantic analysis establishes the information diagnostics need to carry.

## Platform considerations

Compiler source-file and diagnostic paths use `std::filesystem::path`, so internal compiler code does not assume Unix-style or Windows-style separators.

The current Makefile uses Unix shell commands and `g++`. Development on native Windows therefore currently works most naturally through Cygwin or MSYS2. A native Windows build path can be added later without changing the compiler's internal path representation.

## Development philosophy

C SAM is being built incrementally. Each compiler layer should have a clear responsibility and a regression suite before the next layer becomes substantial.

The current priority is:

```text
lex correctly
    ↓
parse correctly
    ↓
represent correctly
    ↓
validate semantically
    ↓
generate output
```

The lexer, parser, value parser, AST, diagnostics, and test/build infrastructure are now at a solid frontend checkpoint. The next major implementation phase is semantic analysis.
