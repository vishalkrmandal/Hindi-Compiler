/* include/semantic.h */
#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"

// Symbol types
typedef enum
{
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION
} SymbolType;

// Symbol structure for the symbol table
typedef struct Symbol
{
    char *name;
    SymbolType type;
    TokenType dataType;    // For variables and function return types
    int paramCount;        // For functions
    TokenType *paramTypes; // For functions
    int scopeDepth;
    struct Symbol *next;
} Symbol;

// Symbol table structure
typedef struct
{
    Symbol *first;
    int scopeDepth;
} SymbolTable;

// Error tracking
typedef struct
{
    int errorCount;
} SemanticContext;

// Initialize the semantic analyzer
void initSemanticAnalyzer(SemanticContext *context, SymbolTable *symbolTable);

// Analyze the AST
bool analyzeProgram(SemanticContext *context, SymbolTable *symbolTable, AstProgram *program);

// Symbol table operations
void initSymbolTable(SymbolTable *table);
Symbol *defineVariable(SymbolTable *table, const char *name, TokenType dataType, int line, int column);
Symbol *defineFunction(SymbolTable *table, const char *name, TokenType returnType,
                       int paramCount, TokenType *paramTypes, int line, int column);
Symbol *resolveSymbol(SymbolTable *table, const char *name);
void beginScope(SymbolTable *table);
void endScope(SymbolTable *table);
void freeSymbolTable(SymbolTable *table);

// Report semantic errors
void semanticError(SemanticContext *context, int line, int column, const char *message);

#endif /* SEMANTIC_H */