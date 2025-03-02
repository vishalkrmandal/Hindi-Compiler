/* include/ast.h */
#ifndef AST_H
#define AST_H

#include "lexer.h"

// AST node types
typedef enum
{
    // Statements
    AST_PROGRAM,         // Root node
    AST_FUNCTION_DECL,   // Function declaration
    AST_VAR_DECL,        // Variable declaration
    AST_BLOCK,           // Block of statements
    AST_IF,              // If statement
    AST_WHILE,           // While statement
    AST_FOR,             // For statement
    AST_RETURN,          // Return statement
    AST_EXPRESSION_STMT, // Expression statement

    // Expressions
    AST_BINARY,     // Binary operation
    AST_UNARY,      // Unary operation
    AST_LITERAL,    // Literal value
    AST_VARIABLE,   // Variable reference
    AST_ASSIGNMENT, // Variable assignment
    AST_CALL,       // Function call
} AstNodeType;

// Forward declaration
typedef struct AstNode AstNode;

// Base node structure
struct AstNode
{
    AstNodeType type;
    int line;
    int column;
};

// Program (the root of the AST)
typedef struct
{
    AstNode base;
    int count;
    int capacity;
    AstNode **declarations;
} AstProgram;

// Variable declaration
typedef struct
{
    AstNode base;
    Token name;
    TokenType varType;    // Type of variable (INT, FLOAT, etc.)
    AstNode *initializer; // Optional
} AstVarDecl;

// Function declaration
typedef struct
{
    AstNode base;
    Token name;
    TokenType returnType;
    int paramCount;
    struct
    {
        Token name;
        TokenType type;
    } *params;
    AstNode *body;
} AstFunctionDecl;

// Block statement
typedef struct
{
    AstNode base;
    int count;
    int capacity;
    AstNode **statements;
} AstBlock;

// If statement
typedef struct
{
    AstNode base;
    AstNode *condition;
    AstNode *thenBranch;
    AstNode *elseBranch; // Optional
} AstIf;

// While statement
typedef struct
{
    AstNode base;
    AstNode *condition;
    AstNode *body;
} AstWhile;

// For statement
typedef struct
{
    AstNode base;
    AstNode *initializer; // Optional
    AstNode *condition;   // Optional
    AstNode *increment;   // Optional
    AstNode *body;
} AstFor;

// Return statement
typedef struct
{
    AstNode base;
    AstNode *value; // Optional for void functions
} AstReturn;

// Expression statement
typedef struct
{
    AstNode base;
    AstNode *expression;
} AstExpressionStmt;

// Binary expression
typedef struct
{
    AstNode base;
    AstNode *left;
    TokenType operator;
    AstNode *right;
} AstBinary;

// Unary expression
typedef struct
{
    AstNode base;
    TokenType operator;
    AstNode *right;
} AstUnary;

// Literal value
typedef struct
{
    AstNode base;
    Token value;
} AstLiteral;

// Variable reference
typedef struct
{
    AstNode base;
    Token name;
} AstVariable;

// Assignment
typedef struct
{
    AstNode base;
    Token name;
    AstNode *value;
} AstAssignment;

// Function call
typedef struct
{
    AstNode base;
    Token name;
    int argCount;
    int capacity;
    AstNode **arguments;
} AstCall;

// Functions to create AST nodes
AstProgram *createProgram();
AstVarDecl *createVarDecl(Token name, TokenType type, AstNode *initializer);
AstFunctionDecl *createFunctionDecl(Token name, TokenType returnType);
AstBlock *createBlock();
AstIf *createIf(AstNode *condition, AstNode *thenBranch, AstNode *elseBranch);
AstWhile *createWhile(AstNode *condition, AstNode *body);
AstFor *createFor(AstNode *initializer, AstNode *condition, AstNode *increment, AstNode *body);
AstReturn *createReturn(AstNode *value);
AstExpressionStmt *createExpressionStmt(AstNode *expression);
AstBinary *createBinary(AstNode *left, TokenType operator, AstNode * right);
AstUnary *createUnary(TokenType operator, AstNode * right);
AstLiteral *createLiteral(Token value);
AstVariable *createVariable(Token name);
AstAssignment *createAssignment(Token name, AstNode *value);
AstCall *createCall(Token name);

// Functions to free AST nodes
void freeAst(AstNode *node);

#endif /* AST_H */