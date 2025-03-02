/* src/ast/ast.c */
#include "../../include/ast.h"
#include <stdlib.h>

// Helper to initialize the base AST node
static void initNode(AstNode *node, AstNodeType type, int line, int column)
{
    node->type = type;
    node->line = line;
    node->column = column;
}

// Create a program node (root of AST)
AstProgram *createProgram()
{
    AstProgram *node = (AstProgram *)malloc(sizeof(AstProgram));
    initNode((AstNode *)node, AST_PROGRAM, 0, 0);
    node->count = 0;
    node->capacity = 8; // Initial capacity
    node->declarations = (AstNode **)malloc(sizeof(AstNode *) * node->capacity);
    return node;
}

// Create a variable declaration node
AstVarDecl *createVarDecl(Token name, TokenType type, AstNode *initializer)
{
    AstVarDecl *node = (AstVarDecl *)malloc(sizeof(AstVarDecl));
    initNode((AstNode *)node, AST_VAR_DECL, name.line, name.column);
    node->name = name;
    node->varType = type;
    node->initializer = initializer;
    return node;
}

// Create a function declaration node
AstFunctionDecl *createFunctionDecl(Token name, TokenType returnType)
{
    AstFunctionDecl *node = (AstFunctionDecl *)malloc(sizeof(AstFunctionDecl));
    initNode((AstNode *)node, AST_FUNCTION_DECL, name.line, name.column);
    node->name = name;
    node->returnType = returnType;
    node->paramCount = 0;
    // Allocate space for parameters (up to 8 parameters)
    node->params = malloc(sizeof(struct { Token name; TokenType type; }) * 8);
    node->body = NULL;
    return node;
}

// Create a block statement node
AstBlock *createBlock()
{
    AstBlock *node = (AstBlock *)malloc(sizeof(AstBlock));
    initNode((AstNode *)node, AST_BLOCK, 0, 0); // Line and column will be set later
    node->count = 0;
    node->capacity = 8; // Initial capacity
    node->statements = (AstNode **)malloc(sizeof(AstNode *) * node->capacity);
    return node;
}

// Create an if statement node
AstIf *createIf(AstNode *condition, AstNode *thenBranch, AstNode *elseBranch)
{
    AstIf *node = (AstIf *)malloc(sizeof(AstIf));
    initNode((AstNode *)node, AST_IF, condition->line, condition->column);
    node->condition = condition;
    node->thenBranch = thenBranch;
    node->elseBranch = elseBranch;
    return node;
}

// Create a while statement node
AstWhile *createWhile(AstNode *condition, AstNode *body)
{
    AstWhile *node = (AstWhile *)malloc(sizeof(AstWhile));
    initNode((AstNode *)node, AST_WHILE, condition->line, condition->column);
    node->condition = condition;
    node->body = body;
    return node;
}

// Create a for statement node
AstFor *createFor(AstNode *initializer, AstNode *condition, AstNode *increment, AstNode *body)
{
    AstFor *node = (AstFor *)malloc(sizeof(AstFor));
    // Initialize with the location of the first non-null component
    int line = 0, column = 0;
    if (initializer)
    {
        line = initializer->line;
        column = initializer->column;
    }
    else if (condition)
    {
        line = condition->line;
        column = condition->column;
    }
    else if (increment)
    {
        line = increment->line;
        column = increment->column;
    }
    else if (body)
    {
        line = body->line;
        column = body->column;
    }

    initNode((AstNode *)node, AST_FOR, line, column);
    node->initializer = initializer;
    node->condition = condition;
    node->increment = increment;
    node->body = body;
    return node;
}

// Create a return statement node
AstReturn *createReturn(AstNode *value)
{
    AstReturn *node = (AstReturn *)malloc(sizeof(AstReturn));
    int line = 0, column = 0;
    if (value)
    {
        line = value->line;
        column = value->column;
    }

    initNode((AstNode *)node, AST_RETURN, line, column);
    node->value = value;
    return node;
}

// Create an expression statement node
AstExpressionStmt *createExpressionStmt(AstNode *expression)
{
    AstExpressionStmt *node = (AstExpressionStmt *)malloc(sizeof(AstExpressionStmt));
    initNode((AstNode *)node, AST_EXPRESSION_STMT, expression->line, expression->column);
    node->expression = expression;
    return node;
}

// Create a binary expression node
AstBinary *createBinary(AstNode *left, TokenType operator, AstNode * right)
{
    AstBinary *node = (AstBinary *)malloc(sizeof(AstBinary));
    initNode((AstNode *)node, AST_BINARY, left->line, left->column);
    node->left = left;
    node->operator= operator;
    node->right = right;
    return node;
}

// Create a unary expression node
AstUnary *createUnary(TokenType operator, AstNode * right)
{
    AstUnary *node = (AstUnary *)malloc(sizeof(AstUnary));
    initNode((AstNode *)node, AST_UNARY, right->line, right->column);
    node->operator= operator;
    node->right = right;
    return node;
}

// Create a literal node
AstLiteral *createLiteral(Token value)
{
    AstLiteral *node = (AstLiteral *)malloc(sizeof(AstLiteral));
    initNode((AstNode *)node, AST_LITERAL, value.line, value.column);
    node->value = value;
    return node;
}

// Create a variable reference node
AstVariable *createVariable(Token name)
{
    AstVariable *node = (AstVariable *)malloc(sizeof(AstVariable));
    initNode((AstNode *)node, AST_VARIABLE, name.line, name.column);
    node->name = name;
    return node;
}

// Create an assignment node
AstAssignment *createAssignment(Token name, AstNode *value)
{
    AstAssignment *node = (AstAssignment *)malloc(sizeof(AstAssignment));
    initNode((AstNode *)node, AST_ASSIGNMENT, name.line, name.column);
    node->name = name;
    node->value = value;
    return node;
}

// Create a function call node
AstCall *createCall(Token name)
{
    AstCall *node = (AstCall *)malloc(sizeof(AstCall));
    initNode((AstNode *)node, AST_CALL, name.line, name.column);
    node->name = name;
    node->argCount = 0;
    node->capacity = 4; // Initial capacity
    node->arguments = (AstNode **)malloc(sizeof(AstNode *) * node->capacity);
    return node;
}

// Free AST nodes
void freeAst(AstNode *node)
{
    if (node == NULL)
        return;

    switch (node->type)
    {
    case AST_PROGRAM:
    {
        AstProgram *program = (AstProgram *)node;
        for (int i = 0; i < program->count; i++)
        {
            freeAst(program->declarations[i]);
        }
        free(program->declarations);
        break;
    }
    case AST_VAR_DECL:
    {
        AstVarDecl *varDecl = (AstVarDecl *)node;
        freeAst(varDecl->initializer);
        break;
    }
    case AST_FUNCTION_DECL:
    {
        AstFunctionDecl *funcDecl = (AstFunctionDecl *)node;
        free(funcDecl->params);
        freeAst(funcDecl->body);
        break;
    }
    case AST_BLOCK:
    {
        AstBlock *block = (AstBlock *)node;
        for (int i = 0; i < block->count; i++)
        {
            freeAst(block->statements[i]);
        }
        free(block->statements);
        break;
    }
    case AST_IF:
    {
        AstIf *ifStmt = (AstIf *)node;
        freeAst(ifStmt->condition);
        freeAst(ifStmt->thenBranch);
        freeAst(ifStmt->elseBranch);
        break;
    }
    case AST_WHILE:
    {
        AstWhile *whileStmt = (AstWhile *)node;
        freeAst(whileStmt->condition);
        freeAst(whileStmt->body);
        break;
    }
    case AST_FOR:
    {
        AstFor *forStmt = (AstFor *)node;
        freeAst(forStmt->initializer);
        freeAst(forStmt->condition);
        freeAst(forStmt->increment);
        freeAst(forStmt->body);
        break;
    }
    case AST_RETURN:
    {
        AstReturn *returnStmt = (AstReturn *)node;
        freeAst(returnStmt->value);
        break;
    }
    case AST_EXPRESSION_STMT:
    {
        AstExpressionStmt *exprStmt = (AstExpressionStmt *)node;
        freeAst(exprStmt->expression);
        break;
    }
    case AST_BINARY:
    {
        AstBinary *binary = (AstBinary *)node;
        freeAst(binary->left);
        freeAst(binary->right);
        break;
    }
    case AST_UNARY:
    {
        AstUnary *unary = (AstUnary *)node;
        freeAst(unary->right);
        break;
    }
    case AST_CALL:
    {
        AstCall *call = (AstCall *)node;
        for (int i = 0; i < call->argCount; i++)
        {
            freeAst(call->arguments[i]);
        }
        free(call->arguments);
        break;
    }
    case AST_LITERAL:
    case AST_VARIABLE:
    case AST_ASSIGNMENT:
        // No sub-nodes to free
        break;
    }

    free(node);
}