/* src/parser/parser.c */
#include "../../include/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations for recursive descent parsing
static AstNode *declaration(Parser *parser);
static AstNode *varDeclaration(Parser *parser, TokenType type);
static AstFunctionDecl *functionDeclaration(Parser *parser, TokenType returnType);
static AstNode *statement(Parser *parser);
static AstBlock *blockStatement(Parser *parser);
static AstNode *ifStatement(Parser *parser);
static AstNode *whileStatement(Parser *parser);
static AstNode *forStatement(Parser *parser);
static AstNode *returnStatement(Parser *parser);
static AstNode *expressionStatement(Parser *parser);
static AstNode *expression(Parser *parser);
static AstNode *assignment(Parser *parser);
static AstNode *logicalOr(Parser *parser);
static AstNode *logicalAnd(Parser *parser);
static AstNode *equality(Parser *parser);
static AstNode *comparison(Parser *parser);
static AstNode *term(Parser *parser);
static AstNode *factor(Parser *parser);
static AstNode *unary(Parser *parser);
static AstNode *call(Parser *parser);
static AstNode *primary(Parser *parser);

// Utility functions
static void advance(Parser *parser);
static bool check(Parser *parser, TokenType type);
static bool match(Parser *parser, TokenType type);
static bool consume(Parser *parser, TokenType type, const char *message);
static void synchronize(Parser *parser);

void initParser(Parser *parser, Lexer *lexer)
{
    parser->lexer = lexer;
    parser->hadError = false;
    parser->panicMode = false;
    advance(parser); // Load the first token
}

void parserError(Parser *parser, const char *message)
{
    if (parser->panicMode)
        return;
    parser->panicMode = true;

    fprintf(stderr, "Line %d, Column %d: Error: %s\n",
            parser->current.line,
            parser->current.column,
            message);

    parser->hadError = true;
}

static void advance(Parser *parser)
{
    parser->previous = parser->current;

    for (;;)
    {
        parser->current = scanToken(parser->lexer);
        if (parser->current.type != TOKEN_ERROR)
            break;

        // Report the error
        parserError(parser, parser->current.start);
    }
}

static bool check(Parser *parser, TokenType type)
{
    return parser->current.type == type;
}

static bool match(Parser *parser, TokenType type)
{
    if (!check(parser, type))
        return false;
    advance(parser);
    return true;
}

static bool consume(Parser *parser, TokenType type, const char *message)
{
    if (check(parser, type))
    {
        advance(parser);
        return true;
    }

    parserError(parser, message);
    return false;
}

static void synchronize(Parser *parser)
{
    parser->panicMode = false;

    while (parser->current.type != TOKEN_EOF)
    {
        if (parser->previous.type == TOKEN_SEMICOLON)
            return;

        switch (parser->current.type)
        {
        case TOKEN_INT:
        case TOKEN_FLOAT:
        case TOKEN_CHAR:
        case TOKEN_VOID:
        case TOKEN_IF:
        case TOKEN_WHILE:
        case TOKEN_FOR:
        case TOKEN_RETURN:
            return;
        default:
            // Do nothing
            ;
        }

        advance(parser);
    }
}

// Parse the entire program
AstProgram *parse(Parser *parser)
{
    AstProgram *program = createProgram();

    while (!check(parser, TOKEN_EOF))
    {
        AstNode *decl = declaration(parser);
        if (decl != NULL)
        {
            // Add the declaration to the program
            if (program->count >= program->capacity)
            {
                program->capacity = program->capacity * 2;
                program->declarations = realloc(program->declarations,
                                                sizeof(AstNode *) * program->capacity);
            }
            program->declarations[program->count++] = decl;
        }
    }

    return program;
}

// Parse a declaration (var or function)
static AstNode *declaration(Parser *parser)
{
    // Check for type specifier
    if (match(parser, TOKEN_INT) || match(parser, TOKEN_FLOAT) ||
        match(parser, TOKEN_CHAR) || match(parser, TOKEN_VOID))
    {
        TokenType type = parser->previous.type;

        // Check for function or variable
        if (check(parser, TOKEN_IDENTIFIER) &&
            (parser->lexer->current[0] == '(' || parser->lexer->current[1] == '('))
        {
            return (AstNode *)functionDeclaration(parser, type);
        }
        else
        {
            return varDeclaration(parser, type);
        }
    }

    return statement(parser);
}

// Parse a variable declaration
static AstNode *varDeclaration(Parser *parser, TokenType type)
{
    Token name;

    if (consume(parser, TOKEN_IDENTIFIER, "Expect variable name."))
    {
        name = parser->previous;
    }
    else
    {
        return NULL;
    }

    // Check for initializer
    AstNode *initializer = NULL;
    if (match(parser, TOKEN_ASSIGN))
    {
        initializer = expression(parser);
    }

    consume(parser, TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
    return (AstNode *)createVarDecl(name, type, initializer);
}

// Parse a function declaration
static AstFunctionDecl *functionDeclaration(Parser *parser, TokenType returnType)
{
    Token name;

    if (consume(parser, TOKEN_IDENTIFIER, "Expect function name."))
    {
        name = parser->previous;
    }
    else
    {
        return NULL;
    }

    consume(parser, TOKEN_LPAREN, "Expect '(' after function name.");

    // Create the function declaration
    AstFunctionDecl *function = createFunctionDecl(name, returnType);

    // Parse parameters
    if (!check(parser, TOKEN_RPAREN))
    {
        do
        {
            // Add parameter capacity if needed
            if (function->paramCount >= 8)
            { // Assuming initial capacity of 8
                parserError(parser, "Too many function parameters.");
                break;
            }

            // Parameter type
            if (match(parser, TOKEN_INT) || match(parser, TOKEN_FLOAT) ||
                match(parser, TOKEN_CHAR))
            {
                TokenType paramType = parser->previous.type;

                // Parameter name
                if (consume(parser, TOKEN_IDENTIFIER, "Expect parameter name."))
                {
                    function->params[function->paramCount].type = paramType;
                    function->params[function->paramCount].name = parser->previous;
                    function->paramCount++;
                }
            }
            else
            {
                parserError(parser, "Expect parameter type.");
            }
        } while (match(parser, TOKEN_COMMA));
    }

    consume(parser, TOKEN_RPAREN, "Expect ')' after parameters.");

    // Parse function body
    consume(parser, TOKEN_LBRACE, "Expect '{' before function body.");
    function->body = (AstNode *)blockStatement(parser);

    return function;
}

// Parse a statement
static AstNode *statement(Parser *parser)
{
    if (match(parser, TOKEN_IF))
    {
        return ifStatement(parser);
    }
    if (match(parser, TOKEN_WHILE))
    {
        return whileStatement(parser);
    }
    if (match(parser, TOKEN_FOR))
    {
        return forStatement(parser);
    }
    if (match(parser, TOKEN_RETURN))
    {
        return returnStatement(parser);
    }
    if (match(parser, TOKEN_LBRACE))
    {
        return (AstNode *)blockStatement(parser);
    }

    return expressionStatement(parser);
}

// Parse a block statement
static AstBlock *blockStatement(Parser *parser)
{
    AstBlock *block = createBlock();

    while (!check(parser, TOKEN_RBRACE) && !check(parser, TOKEN_EOF))
    {
        AstNode *stmt = declaration(parser);
        if (stmt != NULL)
        {
            // Add statement to block
            if (block->count >= block->capacity)
            {
                block->capacity = block->capacity * 2;
                block->statements = realloc(block->statements,
                                            sizeof(AstNode *) * block->capacity);
            }
            block->statements[block->count++] = stmt;
        }
    }

    consume(parser, TOKEN_RBRACE, "Expect '}' after block.");
    return block;
}

// Parse an if statement
static AstNode *ifStatement(Parser *parser)
{
    consume(parser, TOKEN_LPAREN, "Expect '(' after 'if'.");
    AstNode *condition = expression(parser);
    consume(parser, TOKEN_RPAREN, "Expect ')' after if condition.");

    AstNode *thenBranch = statement(parser);
    AstNode *elseBranch = NULL;

    if (match(parser, TOKEN_ELSE))
    {
        elseBranch = statement(parser);
    }

    return (AstNode *)createIf(condition, thenBranch, elseBranch);
}

// Parse a while statement
static AstNode *whileStatement(Parser *parser)
{
    consume(parser, TOKEN_LPAREN, "Expect '(' after 'while'.");
    AstNode *condition = expression(parser);
    consume(parser, TOKEN_RPAREN, "Expect ')' after while condition.");

    AstNode *body = statement(parser);

    return (AstNode *)createWhile(condition, body);
}

// Parse a for statement
static AstNode *forStatement(Parser *parser)
{
    consume(parser, TOKEN_LPAREN, "Expect '(' after 'for'.");

    // Initializer
    AstNode *initializer = NULL;
    if (match(parser, TOKEN_SEMICOLON))
    {
        // No initializer
    }
    else if (match(parser, TOKEN_INT) || match(parser, TOKEN_FLOAT) ||
             match(parser, TOKEN_CHAR))
    {
        initializer = varDeclaration(parser, parser->previous.type);
    }
    else
    {
        initializer = expressionStatement(parser);
    }

    // Condition
    AstNode *condition = NULL;
    if (!check(parser, TOKEN_SEMICOLON))
    {
        condition = expression(parser);
    }
    consume(parser, TOKEN_SEMICOLON, "Expect ';' after loop condition.");

    // Increment
    AstNode *increment = NULL;
    if (!check(parser, TOKEN_RPAREN))
    {
        increment = expression(parser);
    }
    consume(parser, TOKEN_RPAREN, "Expect ')' after for clauses.");

    // Body
    AstNode *body = statement(parser);

    return (AstNode *)createFor(initializer, condition, increment, body);
}

// Parse a return statement
static AstNode *returnStatement(Parser *parser)
{
    AstNode *value = NULL;
    if (!check(parser, TOKEN_SEMICOLON))
    {
        value = expression(parser);
    }

    consume(parser, TOKEN_SEMICOLON, "Expect ';' after return value.");
    return (AstNode *)createReturn(value);
}

// Parse an expression statement
static AstNode *expressionStatement(Parser *parser)
{
    AstNode *expr = expression(parser);
    consume(parser, TOKEN_SEMICOLON, "Expect ';' after expression.");
    return (AstNode *)createExpressionStmt(expr);
}

// Parse an expression
static AstNode *expression(Parser *parser)
{
    return assignment(parser);
}

// Parse an assignment
static AstNode *assignment(Parser *parser)
{
    AstNode *expr = logicalOr(parser);

    if (match(parser, TOKEN_ASSIGN))
    {
        AstNode *value = assignment(parser);

        if (expr->type == AST_VARIABLE)
        {
            Token name = ((AstVariable *)expr)->name;
            return (AstNode *)createAssignment(name, value);
        }

        parserError(parser, "Invalid assignment target.");
    }

    return expr;
}

// Parse logical OR (||)
static AstNode *logicalOr(Parser *parser)
{
    AstNode *expr = logicalAnd(parser);

    while (match(parser, TOKEN_OR))
    {
        TokenType op = parser->previous.type;
        AstNode *right = logicalAnd(parser);
        expr = (AstNode *)createBinary(expr, op, right);
    }

    return expr;
}

// Parse logical AND (&&)
static AstNode *logicalAnd(Parser *parser)
{
    AstNode *expr = equality(parser);

    while (match(parser, TOKEN_AND))
    {
        TokenType op = parser->previous.type;
        AstNode *right = equality(parser);
        expr = (AstNode *)createBinary(expr, op, right);
    }

    return expr;
}

// Parse equality (==, !=)
static AstNode *equality(Parser *parser)
{
    AstNode *expr = comparison(parser);

    while (match(parser, TOKEN_EQUALS) || match(parser, TOKEN_NOT_EQUALS))
    {
        TokenType op = parser->previous.type;
        AstNode *right = comparison(parser);
        expr = (AstNode *)createBinary(expr, op, right);
    }

    return expr;
}

// Parse comparison (<, >, <=, >=)
static AstNode *comparison(Parser *parser)
{
    AstNode *expr = term(parser);

    while (match(parser, TOKEN_LESS) || match(parser, TOKEN_GREATER) ||
           match(parser, TOKEN_LESS_EQ) || match(parser, TOKEN_GREATER_EQ))
    {
        TokenType op = parser->previous.type;
        AstNode *right = term(parser);
        expr = (AstNode *)createBinary(expr, op, right);
    }

    return expr;
}

// Parse term (+, -)
static AstNode *term(Parser *parser)
{
    AstNode *expr = factor(parser);

    while (match(parser, TOKEN_PLUS) || match(parser, TOKEN_MINUS))
    {
        TokenType op = parser->previous.type;
        AstNode *right = factor(parser);
        expr = (AstNode *)createBinary(expr, op, right);
    }

    return expr;
}

// Parse factor (*, /, %)
static AstNode *factor(Parser *parser)
{
    AstNode *expr = unary(parser);

    while (match(parser, TOKEN_MULTIPLY) || match(parser, TOKEN_DIVIDE) ||
           match(parser, TOKEN_MODULO))
    {
        TokenType op = parser->previous.type;
        AstNode *right = unary(parser);
        expr = (AstNode *)createBinary(expr, op, right);
    }

    return expr;
}

// Parse unary (-, !)
static AstNode *unary(Parser *parser)
{
    if (match(parser, TOKEN_MINUS) || match(parser, TOKEN_NOT))
    {
        TokenType op = parser->previous.type;
        AstNode *right = unary(parser);
        return (AstNode *)createUnary(op, right);
    }

    return call(parser);
}

// Parse function call
static AstNode *call(Parser *parser)
{
    AstNode *expr = primary(parser);

    if (match(parser, TOKEN_LPAREN))
    {
        AstCall *call = NULL;

        if (expr->type == AST_VARIABLE)
        {
            Token name = ((AstVariable *)expr)->name;
            call = createCall(name);
        }
        else
        {
            parserError(parser, "Can only call functions.");
            return expr;
        }

        // Parse arguments
        if (!check(parser, TOKEN_RPAREN))
        {
            do
            {
                if (call->argCount >= call->capacity)
                {
                    call->capacity = call->capacity * 2;
                    call->arguments = realloc(call->arguments,
                                              sizeof(AstNode *) * call->capacity);
                }
                call->arguments[call->argCount++] = expression(parser);
            } while (match(parser, TOKEN_COMMA));
        }

        consume(parser, TOKEN_RPAREN, "Expect ')' after arguments.");

        return (AstNode *)call;
    }

    return expr;
}

// Parse primary expression (literal, variable, grouping)
static AstNode *primary(Parser *parser)
{
    if (match(parser, TOKEN_NUMBER))
    {
        return (AstNode *)createLiteral(parser->previous);
    }

    if (match(parser, TOKEN_STRING))
    {
        return (AstNode *)createLiteral(parser->previous);
    }

    if (match(parser, TOKEN_IDENTIFIER))
    {
        return (AstNode *)createVariable(parser->previous);
    }

    if (match(parser, TOKEN_LPAREN))
    {
        AstNode *expr = expression(parser);
        consume(parser, TOKEN_RPAREN, "Expect ')' after expression.");
        return expr;
    }

    parserError(parser, "Expect expression.");
    return NULL;
}