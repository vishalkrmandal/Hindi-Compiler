# HindiC: Comprehensive Project Documentation

## Table of Contents

1. [Project Overview](#project-overview)
2. [File Structure](#file-structure)
3. [Compiler Components](#compiler-components)
   - [Lexical Analyzer (Lexer)](#lexical-analyzer-lexer)
   - [Parser](#parser)
   - [Abstract Syntax Tree (AST)](#abstract-syntax-tree-ast)
   - [Semantic Analyzer](#semantic-analyzer)
   - [Code Generator](#code-generator)
   - [Main Program](#main-program)
4. [Building and Running](#building-and-running)
5. [Example Programs](#example-programs)
6. [Implementation Challenges](#implementation-challenges)
7. [Future Improvements](#future-improvements)

## Project Overview

HindiC is a programming language with Hindi syntax based on the C programming language. It is designed for Hindi speakers who are more comfortable with their native language. The compiler translates Hindi-syntax code into standard C code, which can then be compiled with a conventional C compiler.

The HindiC compiler follows the traditional compilation pipeline:
1. Lexical analysis - Tokenizing the source code
2. Syntax analysis - Parsing the tokens into an Abstract Syntax Tree (AST)
3. Semantic analysis - Type checking and building a symbol table
4. Code generation - Translating the AST to equivalent C code

## File Structure

```
HindiC/
├── src/                   # Source code for the compiler
│   ├── lexer/             # Lexical analyzer
│   │   └── lexer.c        # Implements the lexical analyzer
│   ├── parser/            # Syntax analyzer
│   │   └── parser.c       # Implements the parser
│   ├── ast/               # Abstract Syntax Tree
│   │   └── ast.c          # Implements AST creation and manipulation
│   ├── semantic/          # Semantic analyzer
│   │   ├── semantic.c     # Implements type checking
│   │   └── symbol_table.c # Implements symbol table operations
│   ├── codegen/           # Code generator
│   │   └── codegen.c      # Implements code generation to C
│   └── main.c             # Entry point of the compiler
├── include/               # Header files
│   ├── lexer.h            # Lexer definitions
│   ├── parser.h           # Parser definitions
│   ├── ast.h              # AST definitions
│   ├── semantic.h         # Semantic analyzer definitions
│   └── codegen.h          # Code generator definitions
├── examples/              # Example HindiC programs
│   ├── hello.hc           # Hello World example
│   └── calculator.hc      # Simple calculator example
├── bin/                   # Compiled binaries
│   └── hindic.exe         # The compiler executable
├── obj/                   # Object files
│   ├── lexer.o            # Compiled lexer
│   ├── parser.o           # Compiled parser
│   ├── ast.o              # Compiled AST
│   ├── semantic.o         # Compiled semantic analyzer
│   ├── symbol_table.o     # Compiled symbol table
│   ├── codegen.o          # Compiled code generator
│   └── main.o             # Compiled main program
├── tests/                 # Test cases
├── Makefile               # Build configuration
├── README.md              # Project overview
└── documentation.md       # Detailed documentation
```

## Compiler Components

### Lexical Analyzer (Lexer)

**Files**: 
- `include/lexer.h`
- `src/lexer/lexer.c`

The lexical analyzer (lexer) is responsible for tokenizing the source code. It reads the input character by character and groups them into tokens such as keywords, identifiers, operators, and literals.

#### Key Features:

1. **Token Definition**: Defines various token types including Hindi keywords, operators, and identifiers.

```c
// include/lexer.h
typedef enum {
    TOKEN_EOF = 0,
    
    // Data types
    TOKEN_INT,       // पूर्णांक
    TOKEN_FLOAT,     // दशमलव
    TOKEN_CHAR,      // वर्ण
    TOKEN_VOID,      // शून्य
    
    // Control flow
    TOKEN_IF,        // अगर
    TOKEN_ELSE,      // वरना
    /* More token types... */
} TokenType;
```

2. **Hindi Character Support**: Implements functions to detect and handle UTF-8 encoded Hindi characters.

```c
// src/lexer/lexer.c
static bool isHindiChar(const char* str) {
    // Check if we have a valid UTF-8 3-byte sequence for Hindi
    // Hindi Unicode range is approximately 0x0900 to 0x097F
    if ((unsigned char)str[0] == 0xE0 &&
        ((unsigned char)str[1] == 0xA4 || (unsigned char)str[1] == 0xA5)) {
        return true;
    }
    return false;
}
```

3. **Keyword Mapping**: Maps Hindi keywords to their corresponding token types.

```c
// src/lexer/lexer.c
static const KeywordMap hindiKeywords[] = {
    {"पूर्णांक", TOKEN_INT},
    {"दशमलव", TOKEN_FLOAT},
    {"वर्ण", TOKEN_CHAR},
    {"शून्य", TOKEN_VOID},
    {"अगर", TOKEN_IF},
    /* More keywords... */
    {NULL, 0} // End sentinel
};
```

4. **Token Recognition**: Recognizes and categorizes tokens from the input stream.

```c
// src/lexer/lexer.c
Token scanToken(Lexer* lexer) {
    skipWhitespace(lexer);
    
    lexer->start = lexer->current;
    
    if (isAtEnd(lexer)) return makeToken(lexer, TOKEN_EOF);
    
    char c = advance(lexer);
    
    // Identifiers
    if (isIdentifierStart(c)) return identifier(lexer);
    
    // Numbers
    if (isDigit(c)) return number(lexer);
    
    // Other tokens like operators and punctuation
    switch (c) {
        // Implementation details...
    }
    
    return errorToken(lexer, "Unexpected character.");
}
```

### Parser

**Files**: 
- `include/parser.h`
- `src/parser/parser.c`

The parser takes the tokens produced by the lexer and builds an Abstract Syntax Tree (AST). It implements a recursive descent parser for the grammar of the HindiC language.

#### Key Features:

1. **Parser Context**: Maintains the state of the parser including current and previous tokens.

```c
// include/parser.h
typedef struct {
    Lexer* lexer;
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} Parser;
```

2. **Recursive Descent Functions**: Implements parsing functions for different syntax constructs.

```c
// src/parser/parser.c
static AstNode* declaration(Parser* parser) {
    // Handles variable and function declarations
    if (match(parser, TOKEN_INT) || match(parser, TOKEN_FLOAT) || 
        match(parser, TOKEN_CHAR) || match(parser, TOKEN_VOID)) {
        TokenType type = parser->previous.type;
        
        // Function or variable
        if (check(parser, TOKEN_IDENTIFIER) && 
            lookahead(parser, TOKEN_LPAREN)) {
            return (AstNode*)functionDeclaration(parser, type);
        } else {
            return varDeclaration(parser, type);
        }
    }
    
    return statement(parser);
}
```

3. **Error Handling**: Implements error reporting and recovery mechanisms.

```c
// src/parser/parser.c
static void synchronize(Parser* parser) {
    parser->panicMode = false;
    
    while (parser->current.type != TOKEN_EOF) {
        if (parser->previous.type == TOKEN_SEMICOLON) return;
        
        switch (parser->current.type) {
            // Sync points for recovery
            case TOKEN_INT:
            case TOKEN_FLOAT:
            /* More token types... */
                return;
            default:
                ; // Do nothing
        }
        
        advance(parser);
    }
}
```

### Abstract Syntax Tree (AST)

**Files**: 
- `include/ast.h`
- `src/ast/ast.c`

The Abstract Syntax Tree (AST) represents the hierarchical structure of the parsed program. It consists of various node types for different language constructs.

#### Key Features:

1. **Node Types**: Defines different types of AST nodes.

```c
// include/ast.h
typedef enum {
    // Statements
    AST_PROGRAM,          // Root node
    AST_FUNCTION_DECL,    // Function declaration
    AST_VAR_DECL,         // Variable declaration
    AST_BLOCK,            // Block of statements
    /* More node types... */
} AstNodeType;
```

2. **Node Structures**: Defines the structure for each type of AST node.

```c
// include/ast.h
// Base node structure
struct AstNode {
    AstNodeType type;
    int line;
    int column;
};

// Program (the root of the AST)
typedef struct {
    AstNode base;
    int count;
    int capacity;
    AstNode** declarations;
} AstProgram;

// Function declaration
typedef struct {
    AstNode base;
    Token name;
    TokenType returnType;
    int paramCount;
    struct {
        Token name;
        TokenType type;
    }* params;
    AstNode* body;
} AstFunctionDecl;

/* More node structures... */
```

3. **Node Creation**: Implements functions to create different types of AST nodes.

```c
// src/ast/ast.c
AstProgram* createProgram() {
    AstProgram* node = (AstProgram*)malloc(sizeof(AstProgram));
    initNode((AstNode*)node, AST_PROGRAM, 0, 0);
    node->count = 0;
    node->capacity = 8; // Initial capacity
    node->declarations = (AstNode**)malloc(sizeof(AstNode*) * node->capacity);
    return node;
}

/* More creation functions... */
```

4. **Node Manipulation**: Implements functions to manipulate the AST.

```c
// src/ast/ast.c
void freeAst(AstNode* node) {
    if (node == NULL) return;
    
    switch (node->type) {
        case AST_PROGRAM: {
            AstProgram* program = (AstProgram*)node;
            for (int i = 0; i < program->count; i++) {
                freeAst(program->declarations[i]);
            }
            free(program->declarations);
            break;
        }
        /* More cases... */
    }
    
    free(node);
}
```

### Semantic Analyzer

**Files**: 
- `include/semantic.h`
- `src/semantic/semantic.c`
- `src/semantic/symbol_table.c`

The semantic analyzer performs type checking and builds a symbol table to track variables and functions.

#### Key Features:

1. **Symbol Table**: Implements a symbol table to track variables and functions.

```c
// include/semantic.h
// Symbol types
typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION
} SymbolType;

// Symbol structure for the symbol table
typedef struct Symbol {
    char* name;
    SymbolType type;
    TokenType dataType;  // For variables and function return types
    int paramCount;      // For functions
    TokenType* paramTypes; // For functions
    int scopeDepth;
    struct Symbol* next;
} Symbol;

// Symbol table structure
typedef struct {
    Symbol* first;
    int scopeDepth;
} SymbolTable;
```

2. **Type Checking**: Implements type checking for expressions and statements.

```c
// src/semantic/semantic.c
static TokenType analyzeExpression(SemanticContext* context, SymbolTable* table, AstNode* node) {
    if (node == NULL) return TOKEN_ERROR;
    
    switch (node->type) {
        case AST_BINARY:
            return analyzeBinary(context, table, (AstBinary*)node);
        case AST_UNARY:
            return analyzeUnary(context, table, (AstUnary*)node);
        /* More cases... */
        default:
            semanticError(context, node->line, node->column, "Unknown expression type.");
            return TOKEN_ERROR;
    }
}
```

3. **Symbol Resolution**: Implements looking up symbols in the symbol table.

```c
// src/semantic/symbol_table.c
Symbol* resolveSymbol(SymbolTable* table, const char* name) {
    Symbol* current = table->first;
    
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL; // Not found
}
```

4. **Scope Management**: Implements scope management for variable visibility.

```c
// src/semantic/symbol_table.c
void beginScope(SymbolTable* table) {
    table->scopeDepth++;
}

void endScope(SymbolTable* table) {
    // Remove symbols from the current scope
    Symbol* current = table->first;
    Symbol* previous = NULL;
    
    while (current != NULL) {
        if (current->scopeDepth == table->scopeDepth) {
            // Remove this symbol
            /* Implementation details... */
        } else {
            // Keep this symbol
            previous = current;
            current = current->next;
        }
    }
    
    table->scopeDepth--;
}
```

### Code Generator

**Files**: 
- `include/codegen.h`
- `src/codegen/codegen.c`

The code generator translates the AST into equivalent C code.

#### Key Features:

1. **Code Generation Context**: Maintains the state of the code generator.

```c
// include/codegen.h
typedef struct {
    FILE* output;      // Output file for generated code
    int indentLevel;   // Current indentation level
} CodeGenContext;
```

2. **Node Translation**: Implements functions to translate different types of AST nodes to C code.

```c
// src/codegen/codegen.c
static void generateFunctionDecl(CodeGenContext* context, AstFunctionDecl* node) {
    // Function header
    fprintf(context->output, "%s %.*s(",
           getTypeString(node->returnType),
           node->name.length, node->name.start);
    
    // Parameters
    for (int i = 0; i < node->paramCount; i++) {
        if (i > 0) {
            fprintf(context->output, ", ");
        }
        
        fprintf(context->output, "%s %.*s",
               getTypeString(node->params[i].type),
               node->params[i].name.length, node->params[i].name.start);
    }
    
    fprintf(context->output, ") ");
    
    // Function body
    generateBlock(context, (AstBlock*)node->body);
}
```

3. **Statement Translation**: Implements translation of various statement types.

```c
// src/codegen/codegen.c
static void generateIfStatement(CodeGenContext* context, AstIf* node) {
    emitIndentation(context);
    fprintf(context->output, "if (");
    generateExpression(context, node->condition);
    fprintf(context->output, ") ");
    
    generateStatement(context, node->thenBranch);
    
    if (node->elseBranch != NULL) {
        emitIndentation(context);
        fprintf(context->output, "else ");
        generateStatement(context, node->elseBranch);
    }
}
```

4. **Expression Translation**: Implements translation of expressions.

```c
// src/codegen/codegen.c
static void generateBinary(CodeGenContext* context, AstBinary* node) {
    fprintf(context->output, "(");
    generateExpression(context, node->left);
    
    // Output the operator
    switch (node->operator) {
        case TOKEN_PLUS:
            fprintf(context->output, " + ");
            break;
        /* More operators... */
        default:
            fprintf(stderr, "Unknown binary operator in code generation.\n");
            break;
    }
    
    generateExpression(context, node->right);
    fprintf(context->output, ")");
}
```

### Main Program

**File**: `src/main.c`

The main program ties all the compiler components together and handles command-line arguments.

#### Key Features:

1. **Command-Line Parsing**: Processes command-line arguments to determine compiler behavior.

```c
// src/main.c
int main(int argc, char* argv[]) {
    // Check command-line arguments
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    // Parse command-line options
    char* inputPath = NULL;
    char* outputPath = NULL;
    bool tokenizeOnly = false;
    bool parseOnly = false;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                outputPath = argv[++i];
            } else {
                fprintf(stderr, "Error: -o option requires an argument.\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-t") == 0) {
            tokenizeOnly = true;
        } else if (strcmp(argv[i], "-p") == 0) {
            parseOnly = true;
        } else if (strcmp(argv[i], "-h") == 0) {
            printUsage(argv[0]);
            return 0;
        } else if (inputPath == NULL) {
            inputPath = argv[i];
        } else {
            fprintf(stderr, "Error: Unexpected argument '%s'.\n", argv[i]);
            return 1;
        }
    }
    
    /* More implementation... */
}
```

2. **Compilation Pipeline**: Orchestrates the compilation process from lexical analysis to code generation.

```c
// src/main.c
int main(int argc, char* argv[]) {
    /* Command-line parsing... */
    
    // Read the input file
    char* source = readFile(inputPath);
    if (source == NULL) {
        return 1;
    }
    
    // Initialize the lexer
    Lexer lexer;
    initLexer(&lexer, source);
    
    // Tokenize mode
    if (tokenizeOnly) {
        /* Tokenize and print tokens */
        return 0;
    }
    
    // Initialize the parser
    Parser parser;
    initParser(&parser, &lexer);
    
    // Parse the source code
    AstProgram* program = parse(&parser);
    
    if (parser.hadError) {
        /* Handle parsing error */
        return 1;
    }
    
    if (parseOnly) {
        /* Handle parse-only mode */
        return 0;
    }
    
    // Semantic analysis
    SymbolTable symbolTable;
    SemanticContext semanticContext;
    initSemanticAnalyzer(&semanticContext, &symbolTable);
    
    bool semanticSuccess = analyzeProgram(&semanticContext, &symbolTable, program);
    
    if (!semanticSuccess) {
        /* Handle semantic error */
        return 1;
    }
    
    // Code generation
    FILE* outputFile = fopen(outputPath, "w");
    if (outputFile == NULL) {
        /* Handle output file error */
        return 1;
    }
    
    CodeGenContext codeGenContext;
    initCodeGen(&codeGenContext, outputFile);
    
    generateCode(&codeGenContext, program);
    
    /* Cleanup... */
    
    return 0;
}
```

3. **File Handling**: Manages reading source files and writing output files.

```c
// src/main.c
static char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file '%s'.\n", path);
        return NULL;
    }
    
    // Get file size
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);
    
    // Allocate buffer
    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Not enough memory to read file '%s'.\n", path);
        fclose(file);
        return NULL;
    }
    
    // Read file
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        fprintf(stderr, "Error: Could not read file '%s'.\n", path);
        free(buffer);
        fclose(file);
        return NULL;
    }
    
    // Null-terminate the string
    buffer[bytesRead] = '\0';
    
    fclose(file);
    return buffer;
}
```

## Building and Running

The HindiC compiler can be built from source using the provided Makefile or by manually compiling each component.

### Using Make

```bash
# Build the compiler
make

# Clean build artifacts
make clean

# Test with an example
make test
```

### Manual Compilation on Windows

```powershell
# Create directories if they don't exist
mkdir -Force obj
mkdir -Force bin

# Compile each component
gcc -Wall -Wextra -std=c99 -g -Iinclude -c -o obj/lexer.o src/lexer/lexer.c
gcc -Wall -Wextra -std=c99 -g -Iinclude -c -o obj/parser.o src/parser/parser.c
gcc -Wall -Wextra -std=c99 -g -Iinclude -c -o obj/ast.o src/ast/ast.c
gcc -Wall -Wextra -std=c99 -g -Iinclude -c -o obj/semantic.o src/semantic/semantic.c
gcc -Wall -Wextra -std=c99 -g -Iinclude -c -o obj/symbol_table.o src/semantic/symbol_table.c
gcc -Wall -Wextra -std=c99 -g -Iinclude -c -o obj/codegen.o src/codegen/codegen.c
gcc -Wall -Wextra -std=c99 -g -Iinclude -c -o obj/main.o src/main.c

# Link everything together
gcc -Wall -Wextra -std=c99 -g -o bin/hindic.exe obj/lexer.o obj/parser.o obj/ast.o obj/semantic.o obj/symbol_table.o obj/codegen.o obj/main.o
```

### Running the Compiler

```bash
# Basic usage
./bin/hindic examples/hello.hc -o hello.c

# Tokenize only mode
./bin/hindic examples/hello.hc -t

# Parse only mode
./bin/hindic examples/hello.hc -p

# Compile the generated C code
gcc hello.c -o hello
```

## Example Programs

### Hello World (`examples/hello.hc`)

```
// Hello World program in Hindi-C

शून्य मुख्य() {
    लिखो("नमस्ते दुनिया!");
    वापस 0;
}
```

Generated C code:

```c
#include <stdio.h>
#include <stdlib.h>

void main() {
    printf("नमस्ते दुनिया!");
    return 0;
}
```

### Simple Calculator (`examples/calculator.hc`)

```
// Simple calculator program in Hindi-C

पूर्णांक जोड़(पूर्णांक क, पूर्णांक ख) {
    वापस क + ख;
}

पूर्णांक घटाव(पूर्णांक क, पूर्णांक ख) {
    वापस क - ख;
}

पूर्णांक गुणा(पूर्णांक क, पूर्णांक ख) {
    वापस क * ख;
}

दशमलव भाग(पूर्णांक क, पूर्णांक ख) {
    अगर (ख == 0) {
        लिखो("त्रुटि: शून्य से भाग नहीं कर सकते!");
        वापस 0.0;
    }
    वापस क / ख;
}

पूर्णांक मुख्य() {
    पूर्णांक पहला_संख्या = 10;
    पूर्णांक दूसरा_संख्या = 5;
    
    लिखो("%d + %d = %d\n", पहला_संख्या, दूसरा_संख्या, जोड़(पहला_संख्या, दूसरा_संख्या));
    लिखो("%d - %d = %d\n", पहला_संख्या, दूसरा_संख्या, घटाव(पहला_संख्या, दूसरा_संख्या));
    लिखो("%d * %d = %d\n", पहला_संख्या, दूसरा_संख्या, गुणा(पहला_संख्या, दूसरा_संख्या));
    लिखो("%d / %d = %f\n", पहला_संख्या, दूसरा_संख्या, भाग(पहला_संख्या, दूसरा_संख्या));
    
    वापस 0;
}
```

Generated C code:

```c
#include <stdio.h>
#include <stdlib.h>

int add(int a, int b) {
    return a + b;
}

int subtract(int a, int b) {
    return a - b;
}

int multiply(int a, int b) {
    return a * b;
}

float divide(int a, int b) {
    if (b == 0) {
        printf("Error: Cannot divide by zero!");
        return 0.0;
    }
    return a / (float)b;
}

int main() {
    int first_number = 10;
    int second_number = 5;
    
    printf("%d + %d = %d\n", first_number, second_number, add(first_number, second_number));
    printf("%d - %d = %d\n", first_number, second_number, subtract(first_number, second_number));
    printf("%d * %d = %d\n", first_number, second_number, multiply(first_number, second_number));
    printf("%d / %d = %f\n", first_number, second_number, divide(first_number, second_number));
    
    return 0;
}
```

## Implementation Challenges

### 1. UTF-8 Handling in C

One of the major challenges was dealing with UTF-8 encoded Hindi characters in C, which doesn't natively support Unicode identifiers. Each Hindi character is encoded as a 3-byte sequence in UTF-8, requiring special handling in the lexer.

```c
// Example of how UTF-8 Hindi characters are represented
// "श" is encoded as 0xE0 0xA4 0xB6 (3 bytes)
```

### 2. C Compiler Limitations

Standard C compilers like GCC do not support Hindi identifiers, so the code generator needs to map Hindi identifiers to valid C identifiers.

### 3. Symbol Table with Unicode Support

The symbol table needs to handle Hindi identifiers, which requires proper Unicode comparison and hashing functions.

### 4. Testing with Hindi Input

Testing the compiler with Hindi input requires proper UTF-8 encoding of test files and console output, which can be challenging in environments with limited Unicode support.

## Future Improvements

### 1. Enhanced Unicode Support

Improve Unicode handling throughout the compiler, particularly in the lexer's identifier recognition logic.

### 2. Direct Code Generation

Instead of generating C code, directly generate machine code or LLVM IR to bypass C's Unicode limitations.

### 3. Expanded Standard Library

Create a more comprehensive standard library with Hindi function names for common operations.

### 4. Improved Error Messages

Provide more detailed error messages in Hindi to help users debug their code.

### 5. IDE Integration

Develop plugins for popular IDEs to provide syntax highlighting, code completion, and debugging support for HindiC.#   H i n d i - C o m p i l e r  
 