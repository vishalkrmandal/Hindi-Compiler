/* include/lexer.h */
#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdbool.h>

// Token types
typedef enum
{
    TOKEN_EOF = 0,

    // Data types
    TOKEN_INT,   // पूर्णांक
    TOKEN_FLOAT, // दशमलव
    TOKEN_CHAR,  // वर्ण
    TOKEN_VOID,  // शून्य

    // Control flow
    TOKEN_IF,       // अगर
    TOKEN_ELSE,     // वरना
    TOKEN_FOR,      // दौर
    TOKEN_WHILE,    // जबतक
    TOKEN_DO,       // करो
    TOKEN_BREAK,    // रुको
    TOKEN_CONTINUE, // जारी
    TOKEN_RETURN,   // वापस

    // Literals & Identifiers
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,

    // Operators
    TOKEN_PLUS,       // +
    TOKEN_MINUS,      // -
    TOKEN_MULTIPLY,   // *
    TOKEN_DIVIDE,     // /
    TOKEN_MODULO,     // %
    TOKEN_ASSIGN,     // =
    TOKEN_EQUALS,     // ==
    TOKEN_NOT_EQUALS, // !=
    TOKEN_GREATER,    // >
    TOKEN_LESS,       // <
    TOKEN_GREATER_EQ, // >=
    TOKEN_LESS_EQ,    // <=
    TOKEN_AND,        // &&
    TOKEN_OR,         // ||
    TOKEN_NOT,        // !

    // Punctuation
    TOKEN_SEMICOLON, // ;
    TOKEN_COMMA,     // ,
    TOKEN_LPAREN,    // (
    TOKEN_RPAREN,    // )
    TOKEN_LBRACE,    // {
    TOKEN_RBRACE,    // }

    // Error token
    TOKEN_ERROR
} TokenType;

// Token structure
typedef struct
{
    TokenType type;
    const char *start;
    int length;
    int line;
    int column;
    union
    {
        int int_value;
        float float_value;
        char *string_value;
    } value;
} Token;

// Lexer structure
typedef struct
{
    const char *start;
    const char *current;
    int line;
    int column;
} Lexer;

// Lexer functions
void initLexer(Lexer *lexer, const char *source);
Token scanToken(Lexer *lexer);
const char *getTokenName(TokenType type);

#endif /* LEXER_H */