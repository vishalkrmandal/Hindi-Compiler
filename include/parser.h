/* include/parser.h */
#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct
{
    Lexer *lexer;
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} Parser;

// Initialize the parser
void initParser(Parser *parser, Lexer *lexer);

// Parse the source into an AST
AstProgram *parse(Parser *parser);

// Error reporting
void parserError(Parser *parser, const char *message);

#endif /* PARSER_H */