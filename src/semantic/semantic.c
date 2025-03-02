/* src/semantic/semantic.c */
#include "../../include/semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations for analyzing different AST nodes
static bool analyzeDeclaration(SemanticContext *context, SymbolTable *table, AstNode *node);
static bool analyzeVarDecl(SemanticContext *context, SymbolTable *table, AstVarDecl *node);
static bool analyzeFunctionDecl(SemanticContext *context, SymbolTable *table, AstFunctionDecl *node);
static bool analyzeStatement(SemanticContext *context, SymbolTable *table, AstNode *node);
static bool analyzeBlock(SemanticContext *context, SymbolTable *table, AstBlock *node);
static bool analyzeIfStatement(SemanticContext *context, SymbolTable *table, AstIf *node);
static bool analyzeWhileStatement(SemanticContext *context, SymbolTable *table, AstWhile *node);
static bool analyzeForStatement(SemanticContext *context, SymbolTable *table, AstFor *node);
static bool analyzeReturnStatement(SemanticContext *context, SymbolTable *table, AstReturn *node);
static bool analyzeExpressionStatement(SemanticContext *context, SymbolTable *table, AstExpressionStmt *node);
static TokenType analyzeExpression(SemanticContext *context, SymbolTable *table, AstNode *node);
static TokenType analyzeBinary(SemanticContext *context, SymbolTable *table, AstBinary *node);
static TokenType analyzeUnary(SemanticContext *context, SymbolTable *table, AstUnary *node);
static TokenType analyzeLiteral(SemanticContext *context, SymbolTable *table, AstLiteral *node);
static TokenType analyzeVariable(SemanticContext *context, SymbolTable *table, AstVariable *node);
static TokenType analyzeAssignment(SemanticContext *context, SymbolTable *table, AstAssignment *node);
static TokenType analyzeCall(SemanticContext *context, SymbolTable *table, AstCall *node);

// Current function for return type checking
static TokenType currentFunctionReturnType = TOKEN_VOID;

// Initialize the semantic analyzer
void initSemanticAnalyzer(SemanticContext *context, SymbolTable *symbolTable)
{
    context->errorCount = 0;
    initSymbolTable(symbolTable);
}

// Analyze the program
bool analyzeProgram(SemanticContext *context, SymbolTable *symbolTable, AstProgram *program)
{
    // First pass: Register all global functions and variables
    for (int i = 0; i < program->count; i++)
    {
        AstNode *node = program->declarations[i];

        if (node->type == AST_FUNCTION_DECL)
        {
            AstFunctionDecl *func = (AstFunctionDecl *)node;

            // Count parameters and collect their types
            TokenType *paramTypes = malloc(sizeof(TokenType) * func->paramCount);
            for (int j = 0; j < func->paramCount; j++)
            {
                paramTypes[j] = func->params[j].type;
            }

            // Define the function in the symbol table
            defineFunction(symbolTable, func->name.start, func->returnType,
                           func->paramCount, paramTypes, func->base.line, func->base.column);

            free(paramTypes); // Clean up
        }
    }

    // Second pass: Analyze each declaration
    for (int i = 0; i < program->count; i++)
    {
        if (!analyzeDeclaration(context, symbolTable, program->declarations[i]))
        {
            return false;
        }
    }

    return context->errorCount == 0;
}

// Analyze a declaration
static bool analyzeDeclaration(SemanticContext *context, SymbolTable *table, AstNode *node)
{
    switch (node->type)
    {
    case AST_VAR_DECL:
        return analyzeVarDecl(context, table, (AstVarDecl *)node);
    case AST_FUNCTION_DECL:
        return analyzeFunctionDecl(context, table, (AstFunctionDecl *)node);
    default:
        return analyzeStatement(context, table, node);
    }
}

// Analyze a variable declaration
static bool analyzeVarDecl(SemanticContext *context, SymbolTable *table, AstVarDecl *node)
{
    // Check if the variable has an initializer
    TokenType initType = TOKEN_VOID;
    if (node->initializer != NULL)
    {
        initType = analyzeExpression(context, table, node->initializer);

        // Check type compatibility
        if (initType != node->varType && initType != TOKEN_ERROR)
        {
            semanticError(context, node->base.line, node->base.column,
                          "Type mismatch in variable initialization.");
        }
    }

    // Define the variable in the symbol table
    Symbol *symbol = defineVariable(table, node->name.start, node->varType,
                                    node->base.line, node->base.column);

    return symbol != NULL;
}

// Analyze a function declaration
static bool analyzeFunctionDecl(SemanticContext *context, SymbolTable *table, AstFunctionDecl *node)
{
    // Save the current function return type for return statement checking
    TokenType previousReturnType = currentFunctionReturnType;
    currentFunctionReturnType = node->returnType;

    // Create a new scope for function parameters and body
    beginScope(table);

    // Define parameters in the new scope
    for (int i = 0; i < node->paramCount; i++)
    {
        defineVariable(table, node->params[i].name.start, node->params[i].type,
                       node->params[i].name.line, node->params[i].name.column);
    }

    // Analyze function body
    bool result = analyzeBlock(context, table, (AstBlock *)node->body);

    // End scope and restore previous function return type
    endScope(table);
    currentFunctionReturnType = previousReturnType;

    return result;
}

// Analyze a statement
static bool analyzeStatement(SemanticContext *context, SymbolTable *table, AstNode *node)
{
    switch (node->type)
    {
    case AST_BLOCK:
        return analyzeBlock(context, table, (AstBlock *)node);
    case AST_IF:
        return analyzeIfStatement(context, table, (AstIf *)node);
    case AST_WHILE:
        return analyzeWhileStatement(context, table, (AstWhile *)node);
    case AST_FOR:
        return analyzeForStatement(context, table, (AstFor *)node);
    case AST_RETURN:
        return analyzeReturnStatement(context, table, (AstReturn *)node);
    case AST_EXPRESSION_STMT:
        return analyzeExpressionStatement(context, table, (AstExpressionStmt *)node);
    default:
        semanticError(context, node->line, node->column, "Unknown statement type.");
        return false;
    }
}

// Analyze a block statement
static bool analyzeBlock(SemanticContext *context, SymbolTable *table, AstBlock *node)
{
    beginScope(table);

    for (int i = 0; i < node->count; i++)
    {
        if (!analyzeDeclaration(context, table, node->statements[i]))
        {
            endScope(table);
            return false;
        }
    }

    endScope(table);
    return true;
}

// Analyze an if statement
static bool analyzeIfStatement(SemanticContext *context, SymbolTable *table, AstIf *node)
{
    // Analyze the condition
    TokenType condType = analyzeExpression(context, table, node->condition);

    // Condition should be a boolean expression
    if (condType != TOKEN_ERROR && condType != TOKEN_INT)
    {
        semanticError(context, node->condition->line, node->condition->column,
                      "Condition must be a boolean expression.");
    }

    // Analyze the then branch
    bool thenResult = analyzeStatement(context, table, node->thenBranch);

    // Analyze the else branch if it exists
    bool elseResult = true;
    if (node->elseBranch != NULL)
    {
        elseResult = analyzeStatement(context, table, node->elseBranch);
    }

    return thenResult && elseResult;
}

// Analyze a while statement
static bool analyzeWhileStatement(SemanticContext *context, SymbolTable *table, AstWhile *node)
{
    // Analyze the condition
    TokenType condType = analyzeExpression(context, table, node->condition);

    // Condition should be a boolean expression
    if (condType != TOKEN_ERROR && condType != TOKEN_INT)
    {
        semanticError(context, node->condition->line, node->condition->column,
                      "Condition must be a boolean expression.");
    }

    // Analyze the loop body
    return analyzeStatement(context, table, node->body);
}

// Analyze a for statement
static bool analyzeForStatement(SemanticContext *context, SymbolTable *table, AstFor *node)
{
    beginScope(table);

    // Analyze initializer if present
    if (node->initializer != NULL)
    {
        if (!analyzeDeclaration(context, table, node->initializer))
        {
            endScope(table);
            return false;
        }
    }

    // Analyze condition if present
    if (node->condition != NULL)
    {
        TokenType condType = analyzeExpression(context, table, node->condition);

        // Condition should be a boolean expression
        if (condType != TOKEN_ERROR && condType != TOKEN_INT)
        {
            semanticError(context, node->condition->line, node->condition->column,
                          "Condition must be a boolean expression.");
        }
    }

    // Analyze increment if present
    if (node->increment != NULL)
    {
        analyzeExpression(context, table, node->increment);
    }

    // Analyze the loop body
    bool result = analyzeStatement(context, table, node->body);

    endScope(table);
    return result;
}

// Analyze a return statement
static bool analyzeReturnStatement(SemanticContext *context, SymbolTable *table, AstReturn *node)
{
    // Check if returning from void function without a value
    if (currentFunctionReturnType == TOKEN_VOID && node->value != NULL)
    {
        semanticError(context, node->base.line, node->base.column,
                      "Cannot return a value from a void function.");
        return false;
    }

    // Check if returning from non-void function without a value
    if (currentFunctionReturnType != TOKEN_VOID && node->value == NULL)
    {
        semanticError(context, node->base.line, node->base.column,
                      "Missing return value in non-void function.");
        return false;
    }

    // Check return value type if present
    if (node->value != NULL)
    {
        TokenType valueType = analyzeExpression(context, table, node->value);

        if (valueType != TOKEN_ERROR && valueType != currentFunctionReturnType)
        {
            semanticError(context, node->value->line, node->value->column,
                          "Return type mismatch.");
            return false;
        }
    }

    return true;
}

// Analyze an expression statement
static bool analyzeExpressionStatement(SemanticContext *context, SymbolTable *table, AstExpressionStmt *node)
{
    analyzeExpression(context, table, node->expression);
    return true;
}

// Analyze an expression and return its type
static TokenType analyzeExpression(SemanticContext *context, SymbolTable *table, AstNode *node)
{
    if (node == NULL)
        return TOKEN_ERROR;

    switch (node->type)
    {
    case AST_BINARY:
        return analyzeBinary(context, table, (AstBinary *)node);
    case AST_UNARY:
        return analyzeUnary(context, table, (AstUnary *)node);
    case AST_LITERAL:
        return analyzeLiteral(context, table, (AstLiteral *)node);
    case AST_VARIABLE:
        return analyzeVariable(context, table, (AstVariable *)node);
    case AST_ASSIGNMENT:
        return analyzeAssignment(context, table, (AstAssignment *)node);
    case AST_CALL:
        return analyzeCall(context, table, (AstCall *)node);
    default:
        semanticError(context, node->line, node->column, "Unknown expression type.");
        return TOKEN_ERROR;
    }
}

// Analyze a binary expression
static TokenType analyzeBinary(SemanticContext *context, SymbolTable *table, AstBinary *node)
{
    TokenType leftType = analyzeExpression(context, table, node->left);
    TokenType rightType = analyzeExpression(context, table, node->right);

    // Skip further analysis if either operand had errors
    if (leftType == TOKEN_ERROR || rightType == TOKEN_ERROR)
    {
        return TOKEN_ERROR;
    }

    // For arithmetic operators
    if (node->operator== TOKEN_PLUS || node->operator== TOKEN_MINUS ||
        node->operator== TOKEN_MULTIPLY || node->operator== TOKEN_DIVIDE ||
        node->operator== TOKEN_MODULO)
    {

        // Both operands must be numeric
        if ((leftType != TOKEN_INT && leftType != TOKEN_FLOAT) ||
            (rightType != TOKEN_INT && rightType != TOKEN_FLOAT))
        {
            semanticError(context, node->base.line, node->base.column,
                          "Arithmetic operators require numeric operands.");
            return TOKEN_ERROR;
        }

        // If either operand is float, result is float
        if (leftType == TOKEN_FLOAT || rightType == TOKEN_FLOAT)
        {
            return TOKEN_FLOAT;
        }

        return TOKEN_INT;
    }

    // For comparison operators
    if (node->operator== TOKEN_EQUALS || node->operator== TOKEN_NOT_EQUALS ||
        node->operator== TOKEN_LESS || node->operator== TOKEN_GREATER ||
        node->operator== TOKEN_LESS_EQ || node->operator== TOKEN_GREATER_EQ)
    {

        // Types must be compatible
        if (leftType != rightType)
        {
            semanticError(context, node->base.line, node->base.column,
                          "Comparison operators require compatible operands.");
            return TOKEN_ERROR;
        }

        // Comparison results in a boolean (int)
        return TOKEN_INT;
    }

    // For logical operators (&&, ||)
    if (node->operator== TOKEN_AND || node->operator== TOKEN_OR)
    {
        // Both operands must be boolean-convertible (int)
        if (leftType != TOKEN_INT || rightType != TOKEN_INT)
        {
            semanticError(context, node->base.line, node->base.column,
                          "Logical operators require boolean operands.");
            return TOKEN_ERROR;
        }

        return TOKEN_INT;
    }

    semanticError(context, node->base.line, node->base.column,
                  "Unknown binary operator.");
    return TOKEN_ERROR;
}

// Analyze a unary expression
static TokenType analyzeUnary(SemanticContext *context, SymbolTable *table, AstUnary *node)
{
    TokenType operandType = analyzeExpression(context, table, node->right);

    // Skip further analysis if operand had errors
    if (operandType == TOKEN_ERROR)
    {
        return TOKEN_ERROR;
    }

    // Negation operator (-)
    if (node->operator== TOKEN_MINUS)
    {
        if (operandType != TOKEN_INT && operandType != TOKEN_FLOAT)
        {
            semanticError(context, node->base.line, node->base.column,
                          "Unary negation requires a numeric operand.");
            return TOKEN_ERROR;
        }

        return operandType;
    }

    // Logical NOT operator (!)
    if (node->operator== TOKEN_NOT)
    {
        if (operandType != TOKEN_INT)
        {
            semanticError(context, node->base.line, node->base.column,
                          "Logical NOT requires a boolean operand.");
            return TOKEN_ERROR;
        }

        return TOKEN_INT;
    }

    semanticError(context, node->base.line, node->base.column,
                  "Unknown unary operator.");
    return TOKEN_ERROR;
}

// Analyze a literal
static TokenType analyzeLiteral(SemanticContext *context, SymbolTable *table, AstLiteral *node)
{
    switch (node->value.type)
    {
    case TOKEN_NUMBER:
        // Check if it's an integer or float
        if (strchr(node->value.start, '.') != NULL)
        {
            return TOKEN_FLOAT;
        }
        else
        {
            return TOKEN_INT;
        }
    case TOKEN_STRING:
        return TOKEN_CHAR; // Treating string as character array
    default:
        semanticError(context, node->base.line, node->base.column,
                      "Unknown literal type.");
        return TOKEN_ERROR;
    }
}

// Analyze a variable reference
static TokenType analyzeVariable(SemanticContext *context, SymbolTable *table, AstVariable *node)
{
    Symbol *symbol = resolveSymbol(table, node->name.start);

    if (symbol == NULL)
    {
        semanticError(context, node->base.line, node->base.column,
                      "Undefined variable.");
        return TOKEN_ERROR;
    }

    if (symbol->type != SYMBOL_VARIABLE)
    {
        semanticError(context, node->base.line, node->base.column,
                      "Expected a variable name.");
        return TOKEN_ERROR;
    }

    return symbol->dataType;
}

// Analyze an assignment
static TokenType analyzeAssignment(SemanticContext *context, SymbolTable *table, AstAssignment *node)
{
    TokenType valueType = analyzeExpression(context, table, node->value);

    Symbol *symbol = resolveSymbol(table, node->name.start);

    if (symbol == NULL)
    {
        semanticError(context, node->base.line, node->base.column,
                      "Undefined variable in assignment.");
        return TOKEN_ERROR;
    }

    if (symbol->type != SYMBOL_VARIABLE)
    {
        semanticError(context, node->base.line, node->base.column,
                      "Cannot assign to a function.");
        return TOKEN_ERROR;
    }

    if (valueType != TOKEN_ERROR && valueType != symbol->dataType)
    {
        semanticError(context, node->base.line, node->base.column,
                      "Type mismatch in assignment.");
        return TOKEN_ERROR;
    }

    return valueType;
}

// Analyze a function call
static TokenType analyzeCall(SemanticContext *context, SymbolTable *table, AstCall *node)
{
    Symbol *symbol = resolveSymbol(table, node->name.start);

    if (symbol == NULL)
    {
        semanticError(context, node->base.line, node->base.column,
                      "Undefined function.");
        return TOKEN_ERROR;
    }

    if (symbol->type != SYMBOL_FUNCTION)
    {
        semanticError(context, node->base.line, node->base.column,
                      "Cannot call a variable.");
        return TOKEN_ERROR;
    }

    // Check argument count
    if (node->argCount != symbol->paramCount)
    {
        semanticError(context, node->base.line, node->base.column,
                      "Wrong number of arguments.");
        return TOKEN_ERROR;
    }

    // Check argument types
    for (int i = 0; i < node->argCount; i++)
    {
        TokenType argType = analyzeExpression(context, table, node->arguments[i]);

        if (argType != TOKEN_ERROR && argType != symbol->paramTypes[i])
        {
            semanticError(context, node->arguments[i]->line, node->arguments[i]->column,
                          "Argument type mismatch.");
        }
    }

    return symbol->dataType; // Return the function's return type
}