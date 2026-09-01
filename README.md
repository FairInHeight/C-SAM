# C SAM

C SAM is a C++23 compiler project for a new web language that combines HTML-style document structure and CSS styling into a single source file.

The project is intentionally being built in small compiler stages. The current compiler can tokenize C SAM, validate its basic grammar, construct a structural AST, and begin converting CSS-style values into semantic AST value nodes.

The language is still being designed. `test.csam` is the current de facto grammar reference.

## Project goals

C SAM is designed to:

- Combine HTML and CSS into one source file per page.
- Be immediately understandable to people familiar with web development.
- Provide a natural bridge from the C-family of languages into web development.
- Keep the syntax small, predictable, and easy to parse.
- Preserve useful source locations throughout the compiler for clear diagnostics.
- Keep compiler filesystem handling platform-independent by using the C++ standard filesystem library rather than hard-coded Unix or Windows path syntax.
- Eventually compile one C SAM source into generated HTML and CSS.

## Current compiler pipeline

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
    | Grammar validation + nesting + semantic value parsing
    v
   AST
    |
    | Future
    v
CSS / HTML generators
```

The compiler is written in C++23 and is built with the project's Makefile. The compiler's source-file path handling is platform-independent, but the current Makefile uses standard Unix shell commands; native Windows builds may therefore require an environment such as MSYS2 or Cygwin until a native Windows build path is added.

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
│   │   └── value.hpp
│   └── src
│       ├── args.cpp
│       ├── ast.cpp
│       ├── debug.cpp
│       ├── lexer.cpp
│       ├── main.cpp
│       ├── parser.cpp
│       └── value.cpp
├── tests
│   ├── test_ast.cpp
│   ├── test_lexer.cpp
│   └── test_parser.cpp
├── test.csam
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

An empty/structural tag:

```csam
header
{
}
```

A tag with content:

```csam
h1 <"Welcome to my website">
```

A tag may have both content and a block:

```csam
h1 <"Welcome">
{
    font-size: 36px;
}
```

Tags can be nested to arbitrary depth.

### Tag content

Angle brackets `< >` delimit tag content. The first version of the grammar treats content as a sequence of tokens; strings are the primary supported content form today.

```csam
p
<
    "This is my first C-SAM website.\n"
    "It is beautiful, elegant, and predictable.\n"
>
```

Using `< >` deliberately distinguishes tag content from CSS-style declarations and leaves room for richer content, including scripts, in future versions.

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

Properties now pass their values through the semantic value parser. Simple values are represented by dedicated AST nodes, while syntax that has not yet received a semantic representation can remain a raw value.

Examples:

```csam
body
{
    margin: 40px auto;
    opacity: 0.5;
    width: 50%;
    background: mycolor;
}
```

The parser does not currently determine whether a CSS property or CSS unit is valid. Semantic parsing describes the shape of the value; CSS validation will come later.

### Variables

Variables use a simple C-like declaration form:

```csam
var mycolor = #8800ff;
```

Variable values use the same semantic value parsing machinery as properties. Variable resolution, type checking, and semantic scope rules are not implemented yet.

Variables are currently accepted wherever the grammar permits declarations without imposing a full semantic scope system.

## Semantic values

C SAM keeps the lexer granular and lets the parser construct semantic value nodes.

For example:

```text
10
```

becomes a number value, while:

```text
10px
```

is lexed as:

```text
Number("10")
Identifier("px")
```

and interpreted by the parser as a dimension value.

The current semantic value layer contains:

```text
ValueNode
├── NumberValueNode
├── DimensionValueNode
├── PercentageValueNode
├── StringValueNode
└── RawValueNode
```

The separation is intentional:

```text
characters → tokens → semantic values → AST
```

The lexer recognizes lexical units; the parser decides when those tokens form a meaningful CSS-oriented value.

Numeric text is currently preserved rather than immediately converted to a floating-point type. This avoids unnecessary loss of source representation and leaves numeric validation/normalization decisions for later compiler stages.

A dimension requires the number and unit to be adjacent in the source. For example:

```csam
width: 10px;
```

is a dimension, while:

```csam
width: 10 px;
```

is not combined into a dimension merely because whitespace tokens are omitted.

## Functions and nested values

The parser now has the foundation for semantic function values and nested argument collections.

A function is recognized when an identifier is immediately followed by `(`:

```csam
width: calc(100% - 20px);
```

Function arguments are represented as collections of semantic values, with comma-separated arguments kept distinct. Nested functions can therefore be represented recursively.

Whitespace matters for function recognition:

```text
calc(100%)
```

is function syntax, while:

```text
calc (100%)
```

does not become a function solely because the identifier and parenthesis appear next to each other in the token stream.

Function parsing is still an early-stage implementation. CSS-specific functions such as `calc()`, `var()`, color functions, URLs, and gradients are not individually validated yet.

## Basic grammar

The current parser is intentionally a basic grammar checker with an emerging semantic value layer.

Conceptually:

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
    -> var IDENTIFIER = value ;

property
    -> IDENTIFIER : value ;

tag
    -> IDENTIFIER block
    -> IDENTIFIER content
    -> IDENTIFIER content block

content
    -> < content_tokens >

value
    -> semantic_value*
```

A tag name must be followed by `{` or `<`. A property name must be followed by `:`. This distinction is intentional: C SAM uses raw identifier names for tags while retaining familiar CSS punctuation for styling declarations.

## Lexing

The lexer is responsible for converting source characters into tokens. It recognizes the current C SAM lexical vocabulary, including:

- Identifiers, including CSS-style custom-property names and Unicode identifiers
- Strings
- Numbers, percentages, decimal forms, and exponent forms
- Hash values
- At-keywords
- `:`, `;`, `,`, and `=`
- `{}`, `[]`, and `()`
- CSS-oriented punctuation such as `+`, `-`, `*`, `/`, `<`, `>`, `~`, `|`, `^`, `$`, `&`, `!`, `?`, `.`, and `\\`
- CSS selector match operators such as `~=`, `|=`, `^=`, `$=`, `*=` and `||`
- End-of-file
- C-style line and block comments

Malformed exponent forms are handled transactionally. For example, the incomplete exponent in `1e-` does not become part of the number; the lexer leaves the `e` available to be tokenized as an identifier and the `-` as a minus token.

`<` and `>` have a single lexical identity as `LessThan` and `GreaterThan`. Their meaning is determined by parser context: HTML-style tag content uses them as delimiters, while CSS selector grammar can use `>` as a combinator and `<` as ordinary punctuation where applicable. The lexer does not maintain separate HTML and CSS token types for angle brackets.

Tokens carry a shared `SourceLocation` containing:

```text
filepath
line
column
```

`filepath` is represented internally as `std::filesystem::path`, so compiler diagnostics and file access do not assume Unix `/` paths or Windows `\\` paths. The standard library handles native path representation for the host operating system.

The lexer checks lexical constructs such as unterminated strings and unterminated block comments. It does not perform structural delimiter matching.

Whitespace is intentionally not emitted as a token. The parser uses source locations when adjacency matters, such as deciding whether a number is directly followed by a unit or whether an identifier is immediately followed by `(` for function syntax.

## Parser

The parser uses recursive-descent style parsing and is responsible for grammar, structure, and the first level of semantic value construction.

It currently:

- Requires `:root` at the beginning of every file.
- Parses variables, properties, tags, and tag content.
- Supports arbitrary tag nesting.
- Maintains a scope stack while parsing nested tags.
- Checks matching `{}`, `[]`, and `()` delimiters.
- Handles `<` and `>` according to the active grammar context rather than treating them as generic paired delimiters.
- Parses simple numeric, dimension, percentage, and string values into semantic AST nodes.
- Parses function calls into nested semantic value structures.
- Preserves unsupported value syntax through raw value nodes rather than requiring every CSS construct to be modeled immediately.
- Reports source-aware syntax errors.
- Builds the AST while parsing.

Delimiter matching is parser responsibility because the parser understands the syntactic meaning of the delimiters. The lexer only identifies the punctuation as tokens.

The parser's scope stack is separate from delimiter validation. The delimiter stack answers whether punctuation is properly paired; the scope stack answers which C SAM element is currently being parsed. The scope stack contains non-owning pointers to AST nodes owned by the tree.

## AST

The AST now contains both structural document nodes and semantic value nodes:

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
├── RawValueNode
└── FunctionValueNode
```

Every AST node stores a `SourceLocation` describing where the construct originated in the source.

The AST uses `std::unique_ptr` for ownership. The AST owns its nodes; the parser does not own AST nodes and only keeps non-owning pointers to active scope nodes while parsing.

`RootNode` and `TagNode` each maintain one ordered child collection of `ASTNode` objects. This preserves the original sibling order between variables, properties, content, and nested tags instead of grouping those node types into separate lists.

`TagNode` also keeps a non-owning pointer to its `ContentNode` for convenient access. The content node itself is owned by the tag's ordered child collection.

Properties and variables now store semantic `ValueNode` objects rather than only raw token vectors. A raw fallback value is retained for constructs that have not yet received a dedicated semantic representation.

Conceptually, a document such as:

```csam
:root
{
    var mycolor = #8800ff;

    header
    {
        h1 <"Hello">
        {
            color: mycolor;
            padding: 10px;
        }
    }
}
```

becomes structurally similar to:

```text
Root
├── Variable: mycolor
│   └── Raw/semantic value: #8800ff
└── Tag: header
    └── Tag: h1
        ├── Content: "Hello"
        ├── Property: color
        │   └── RawValue: mycolor
        └── Property: padding
            └── DimensionValue: 10px
```

The AST preserves source order within each scope. Semantic values are represented separately from the lexical token stream so future generators can operate on meaning rather than reconstructing the source token-by-token.

The AST does not currently perform CSS property validation, HTML validation, variable resolution, or final code generation.

## Debug mode

Debug output is controlled by the global `csam_debug` flag and is enabled with the short `-d` compiler flag.

Normal compilation is intentionally quiet when successful:

```text
./csam test.csam
```

Debug mode exposes the compiler's working state:

```text
./csam -d test.csam
```

Debug output currently includes lexer progress, the token stream, parser progress, and the constructed AST. The AST debug output follows the same ordered tree structure used by the AST itself.

## Command-line arguments

The compiler accepts one or more source file paths, with an optional flags argument before the first filepath.

Examples:

```text
./csam test.csam
./csam -d test.csam
./csam test.csam other.csam
./csam -d test.csam other.csam
```

The first argument after the executable may be a flags argument when it begins with `-`; otherwise it is treated as the first filepath. Additional arguments are treated as additional filepaths.

Currently supported flag:

```text
-d    Enable compiler debug output
```

Invalid argument syntax and invalid flags produce non-zero exit codes.

## Building and testing

The Makefile provides separate build, test, cleanup, and convenience targets.

Build the compiler:

```text
make
```

Run the test suite:

```text
make test
```

Remove compiler and test binaries:

```text
make clean
```

Clean first, then build and run the tests:

```text
make clean-test
```

Run the full development cycle used for the current debug fixture:

```text
make auto
```

The `auto` target performs:

```text
make clean
make
./csam -d test.csam
make test
```

The repository has automated front-end tests organized by compiler stage:

```text
Lexer tests
    ↓
Parser tests
    ↓
AST tests
```

A successful test run reports:

```text
Running lexer tests...
Lexer tests: PASS
Running parser tests...
Parser tests: PASS
Running AST tests...
AST tests: PASS
All tests passed.
```

`test.csam` remains the current valid grammar reference and integration fixture. `bad.csam` remains a deliberately malformed syntax fixture.

The automated tests are the primary regression mechanism for the compiler front end; the fixtures remain useful as human-readable language examples and parser inputs.

## Current design principles

1. **One source file per page.** HTML-style structure and CSS-style declarations live together.
2. **CSS familiarity.** Styling keeps familiar CSS declarations and punctuation.
3. **C-family familiarity.** Variables use a simple C-like declaration syntax, comments follow C conventions, and the compiler itself is written in C++.
4. **Tags are raw identifiers.** Ordinary tags do not use a colon.
5. **`{}` defines tag/block structure.** Every new block belongs to a tag.
6. **`<>` defines tag content.** This is distinct from CSS declarations and is intentionally extensible.
7. **`:` belongs to declarations.** It separates a property name from its value.
8. **`;` terminates declarations.** Properties and variables require it.
9. **The lexer stays lexical.** It identifies tokens and validates lexical constructs but does not interpret grammar or semantic values.
10. **The parser owns grammar.** It validates structure, tracks nesting, and constructs semantic values.
11. **Angle brackets are contextual.** `<` and `>` have one lexical representation; HTML and CSS grammar determine their meaning.
12. **Semantic values belong in the AST.** Numbers, dimensions, percentages, strings, and functions are represented independently from raw tokens where the language has defined their meaning.
13. **Raw fallbacks are intentional.** Unsupported CSS constructs should remain representable without forcing premature semantic modeling.
14. **The AST preserves source order.** Sibling declarations and tags remain in the order they appeared in the source.
15. **Ownership stays explicit.** AST ownership uses `std::unique_ptr`; parser scope tracking is non-owning.
16. **Whitespace is not a semantic token.** The parser uses source locations when source adjacency affects interpretation.
17. **Filesystem handling stays platform-independent.** Compiler path operations use `std::filesystem::path` rather than hard-coded path separators.
18. **Don't over-engineer early.** The first compiler stages should remain simple and easy to reason about.

## Current status

The lexer, parser, AST, and semantic value foundation are covered by automated tests. The current front end supports structural C SAM documents, CSS-style declarations, C-like variables, Unicode-aware lexical identifiers, CSS-oriented punctuation, semantic primitive values, and early function-value parsing.

The compiler does not yet generate HTML or CSS. Semantic validation, richer selector parsing, complete CSS value validation, variable resolution, and code generation remain future work.

## Next stage

The next planned compiler work is to complete the value system around functions and collections, including robust handling of nested values, comma-separated lists, operators, and CSS function arguments. After that, the project can move into a dedicated selector AST and the first CSS generator.
