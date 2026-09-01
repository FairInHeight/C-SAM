# C SAM

C SAM is a C++23 compiler project for a new web language that combines HTML-style document structure and CSS styling into a single source file.

The project is intentionally being built in small compiler stages. The current compiler can tokenize C SAM, validate its basic grammar, and construct an AST.

## Project goals

C SAM is designed to:

- Combine HTML and CSS into one source file per page.
- Be immediately understandable to people familiar with web development.
- Provide a natural bridge from the C-family of languages into web development.
- Keep the syntax small, predictable, and easy to parse.
- Preserve useful source locations throughout the compiler for clear diagnostics.
- Keep compiler filesystem handling platform-independent by using the C++ standard filesystem library rather than hard-coded Unix or Windows path syntax.

The language is still being designed. `test.csam` is the current de facto grammar reference.

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
    | Grammar validation + nesting + AST construction
    v
   AST
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
│   │   └── token.hpp
│   └── src
│       ├── args.cpp
│       ├── ast.cpp
│       ├── debug.cpp
│       ├── lexer.cpp
│       ├── main.cpp
│       └── parser.cpp
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

A tag is an identifier followed immediately by either `{` or `<`.

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

The word before `{` or `<` is the tag name. Tags can be nested to arbitrary depth.

### Tag content

Angle brackets `< >` delimit tag content. The first version of the grammar treats content as a sequence of tokens; strings are the primary supported content form today. String contents are currently preserved as lexed text; escape sequences are not interpreted by the lexer yet.

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

The parser validates the structure of a property but does not try to determine whether a CSS value is semantically valid. For example, it does not currently know whether a color, font, or CSS property is valid.

Property values are preserved as token sequences so values such as these work naturally:

```csam
margin: 40px auto;
font-family: Arial, sans-serif;
background: mycolor;
```

### Variables

Variables use a simple C-like declaration form:

```csam
var mycolor = #8800ff;
```

The parser currently treats the right-hand side as a token sequence and does not perform type checking or variable resolution.

Variables are currently accepted inside parser blocks without imposing semantic scope rules. Those rules can be added later when semantic analysis exists.

## Basic grammar

The current parser is intentionally a basic grammar checker rather than a semantic analyzer.

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
    -> token*
```

A tag name must be followed by `{` or `<`. A property name must be followed by `:`. This distinction is intentional: C SAM uses raw identifier names for tags while retaining familiar CSS punctuation for styling declarations.

## Lexing

The lexer is responsible for converting source characters into tokens. It recognizes the current C SAM lexical vocabulary, including:

- Identifiers, including CSS-style custom-property names and Unicode identifiers
- Strings
- Numbers and percentages
- Hash values
- At-keywords
- `:`, `;`, `,`, and `=`
- `{}`, `[]`, and `()`
- CSS-oriented punctuation such as `+`, `-`, `*`, `/`, `<`, `>`, `~`, `|`, `^`, `$`, `&`, `!`, `?`, `.`, and `\\`
- CSS selector match operators such as `~=`, `|=`, `^=`, `$=`, `*=` and `||`
- End-of-file
- C-style line and block comments

`<` and `>` have a single lexical identity as `LessThan` and `GreaterThan`. Their meaning is determined by parser context: HTML-style tag content uses them as delimiters, while CSS selector grammar can use `>` as a combinator and `<` as ordinary punctuation where applicable. The lexer does not maintain separate HTML and CSS token types for angle brackets.

Tokens carry a shared `SourceLocation` containing:

```text
filepath
line
column
```

`filepath` is represented internally as `std::filesystem::path`, so compiler diagnostics and file access do not assume Unix `/` paths or Windows `\\` paths. The standard library handles native path representation for the host operating system.

The lexer checks lexical constructs such as unterminated strings and unterminated block comments. It does not perform structural delimiter matching.

## Parser

The parser uses recursive-descent style parsing and is responsible for grammar and structure.

It currently:

- Requires `:root` at the beginning of every file.
- Parses variables, properties, tags, and tag content.
- Supports arbitrary tag nesting.
- Maintains a scope stack while parsing nested tags.
- Checks matching `{}`, `[]`, and `()` delimiters.
- Handles `<` and `>` according to the active grammar context rather than treating them as generic paired delimiters.
- Reports source-aware syntax errors.
- Builds the AST while parsing.

Delimiter matching is parser responsibility because the parser understands the syntactic meaning of the delimiters. The lexer only identifies the punctuation as tokens.

The parser's scope stack is separate from delimiter validation. The delimiter stack answers whether punctuation is properly paired; the scope stack answers which C SAM element is currently being parsed. The scope stack contains non-owning pointers to AST nodes owned by the tree.

## AST

The first AST is intentionally small and structural. It currently contains:

```text
ASTNode
├── RootNode
├── TagNode
├── ContentNode
├── PropertyNode
└── VariableNode
```

Every AST node stores a `SourceLocation` describing where the construct originated in the source.

The AST uses `std::unique_ptr` for ownership. The AST owns its nodes; the parser does not own AST nodes and only keeps non-owning pointers to active scope nodes while parsing.

`RootNode` and `TagNode` each maintain one ordered child collection of `ASTNode` objects. This preserves the original sibling order between variables, properties, content, and nested tags instead of grouping those node types into separate lists.

`TagNode` also keeps a non-owning pointer to its `ContentNode` for convenient access. The content node itself is owned by the tag's ordered child collection.

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
        }
    }
}
```

becomes:

```text
Root
├── Variable: mycolor = #8800ff
└── Tag: header
    └── Tag: h1
        ├── Content: "Hello"
        └── Property: color = mycolor
```

The AST preserves source order within each scope. Values and tag content remain token sequences at this stage rather than being interpreted semantically.

The AST does not currently perform semantic analysis, CSS validation, HTML validation, variable resolution, or code generation.

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

## Testing

The repository now has an automated front-end test harness. The tests are organized by compiler stage:

```text
Lexer tests
    ↓
Parser tests
    ↓
AST tests
```

Run the complete suite with:

```text
make test
```

The harness currently verifies the token stream, parser behavior, and AST construction. A successful run reports:

```text
Running lexer tests...
Lexer tests: PASS
Running parser tests...
Parser tests: PASS
Running AST tests...
AST tests: PASS
All tests passed.
```

`test.csam` remains the current valid grammar reference and should continue to evolve with the language design. `bad.csam` remains a deliberately malformed syntax fixture.

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
9. **The lexer stays lexical.** It identifies tokens and validates lexical constructs but does not interpret grammar.
10. **The parser owns grammar.** It validates structure, tracks nesting, and builds the AST.
11. **Angle brackets are contextual.** `<` and `>` have one lexical representation; HTML and CSS grammar determine their meaning.
12. **The AST stays structural.** Semantic validation and code generation come later.
13. **The AST preserves source order.** Sibling declarations and tags remain in the order they appeared in the source.
14. **Ownership stays explicit.** AST ownership uses `std::unique_ptr`; parser scope tracking is non-owning.
15. **Filesystem handling stays platform-independent.** Compiler path operations use `std::filesystem::path` rather than hard-coded path separators.
16. **Don't over-engineer early.** The first compiler stages should remain simple and easy to reason about.

## Next stage

The lexical, parsing, and first-AST foundations are now covered by automated tests. The next major compiler task is to formalize and implement CSS-compatible numeric lexing, including signs, decimal forms, exponents, percentages, malformed-number handling, and the interaction between numbers and surrounding CSS units/tokens.

Semantic analysis and code generation will come later, after the front-end grammar is sufficiently stable.
