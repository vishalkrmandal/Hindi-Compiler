/* src/codegen/codegen.c */
#include "../../include/codegen.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

// Forward declarations
static void generateDeclaration(CodeGenContext *context, AstNode *node);
static void generateVarDecl(CodeGenContext *context, AstVarDecl *node);
static void generateFunctionDecl(CodeGenContext *context, AstFunctionDecl *node);
static void generateStatement(CodeGenContext *context, AstNode *node);
static void generateBlock(CodeGenContext *context, AstBlock *node);
static void generateIfStatement(CodeGenContext *context, AstIf *node);
static void generateWhileStatement(CodeGenContext *context, AstWhile *node);
static void generateForStatement(CodeGenContext *context, AstFor *node);
static void generateReturnStatement(CodeGenContext *context, AstReturn *node);
static void generateExpressionStatement(CodeGenContext *context, AstExpressionStmt *node);
static void generateExpression(CodeGenContext *context, AstNode *node);
static void generateBinary(CodeGenContext *context, AstBinary *node);
static void generateUnary(CodeGenContext *context, AstUnary *node);
static void generateLiteral(CodeGenContext *context, AstLiteral *node);
static void generateVariable(CodeGenContext *context, AstVariable *node);
static void generateAssignment(CodeGenContext *context, AstAssignment *node);
static void generateCall(CodeGenContext *context, AstCall *node);

// Type conversion from Hindi to C
static const char *getTypeString(TokenType type)
{
    switch (type)
    {
    case TOKEN_INT:
        return "int";
    case TOKEN_FLOAT:
        return "float";
    case TOKEN_CHAR:
        return "char";
    case TOKEN_VOID:
        return "void";
    default:
        return "void"; // Default
    }
}

// Initialize the code generator
void initCodeGen(CodeGenContext *context, FILE *output)
{
    context->output = output;
    context->indentLevel = 0;
}

// Generate indentation
void emitIndentation(CodeGenContext *context)
{
    for (int i = 0; i < context->indentLevel; i++)
    {
        fprintf(context->output, "    "); // 4 spaces per indentation level
    }
}

// Emit a line of code with proper indentation
void emitLine(CodeGenContext *context, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    emitIndentation(context);
    vfprintf(context->output, format, args);
    fprintf(context->output, "\n");

    va_end(args);
}

// Generate code from AST
void generateCode(CodeGenContext *context, AstProgram *program)
{
    // Add standard includes
    fprintf(context->output, "#include <stdio.h>\n");
    fprintf(context->output, "#include <stdlib.h>\n\n");

    // Generate code for each declaration
    for (int i = 0; i < program->count; i++)
    {
        generateDeclaration(context, program->declarations[i]);
        fprintf(context->output, "\n");
    }
}

// Generate code for a declaration
static void generateDeclaration(CodeGenContext *context, AstNode *node)
{
    switch (node->type)
    {
    case AST_VAR_DECL:
        generateVarDecl(context, (AstVarDecl *)node);
        break;
    case AST_FUNCTION_DECL:
        generateFunctionDecl(context, (AstFunctionDecl *)node);
        break;
    default:
        generateStatement(context, node);
        break;
    }
}

// Generate code for a variable declaration
static void generateVarDecl(CodeGenContext *context, AstVarDecl *node)
{
    emitIndentation(context);
    fprintf(context->output, "%s %.*s",
            getTypeString(node->varType),
            node->name.length, node->name.start);

    // If there's an initializer
    if (node->initializer != NULL)
    {
        fprintf(context->output, " = ");
        generateExpression(context, node->initializer);
    }

    fprintf(context->output, ";\n");
}

// Generate code for a function declaration
static void generateFunctionDecl(CodeGenContext *context, AstFunctionDecl *node)
{
    // Function header
    fprintf(context->output, "%s %.*s(",
            getTypeString(node->returnType),
            node->name.length, node->name.start);

    // Parameters
    for (int i = 0; i < node->paramCount; i++)
    {
        if (i > 0)
        {
            fprintf(context->output, ", ");
        }

        fprintf(context->output, "%s %.*s",
                getTypeString(node->params[i].type),
                node->params[i].name.length, node->params[i].name.start);
    }

    fprintf(context->output, ") ");

    // Function body
    generateBlock(context, (AstBlock *)node->body);
}

// Generate code for a statement
static void generateStatement(CodeGenContext *context, AstNode *node)
{
    switch (node->type)
    {
    case AST_BLOCK:
        generateBlock(context, (AstBlock *)node);
        break;
    case AST_IF:
        generateIfStatement(context, (AstIf *)node);
        break;
    case AST_WHILE:
        generateWhileStatement(context, (AstWhile *)node);
        break;
    case AST_FOR:
        generateForStatement(context, (AstFor *)node);
        break;
    case AST_RETURN:
        generateReturnStatement(context, (AstReturn *)node);
        break;
    case AST_EXPRESSION_STMT:
        generateExpressionStatement(context, (AstExpressionStmt *)node);
        break;
    default:
        fprintf(stderr, "Unknown statement type in code generation.\n");
        break;
    }
}

// Generate code for a block statement
static void generateBlock(CodeGenContext *context, AstBlock *node)
{
    emitIndentation(context);
    fprintf(context->output, "{\n");

    context->indentLevel++;
    for (int i = 0; i < node->count; i++)
    {
        generateDeclaration(context, node->statements[i]);
    }
    context->indentLevel--;

    emitIndentation(context);
    fprintf(context->output, "}\n");
}

// Generate code for an if statement
static void generateIfStatement(CodeGenContext *context, AstIf *node)
{
    emitIndentation(context);
    fprintf(context->output, "if (");
    generateExpression(context, node->condition);
    fprintf(context->output, ") ");

    generateStatement(context, node->thenBranch);

    if (node->elseBranch != NULL)
    {
        emitIndentation(context);
        fprintf(context->output, "else ");
        generateStatement(context, node->elseBranch);
    }
}

// Generate code for a while statement
static void generateWhileStatement(CodeGenContext *context, AstWhile *node)
{
    emitIndentation(context);
    fprintf(context->output, "while (");
    generateExpression(context, node->condition);
    fprintf(context->output, ") ");

    generateStatement(context, node->body);
}

// Generate code for a for statement
static void generateForStatement(CodeGenContext *context, AstFor *node)
{
    emitIndentation(context);
    fprintf(context->output, "for (");

    // Initializer
    if (node->initializer != NULL)
    {
        if (node->initializer->type == AST_VAR_DECL)
        {
            AstVarDecl *varDecl = (AstVarDecl *)node->initializer;
            fprintf(context->output, "%s %.*s",
                    getTypeString(varDecl->varType),
                    varDecl->name.length, varDecl->name.start);

            if (varDecl->initializer != NULL)
            {
                fprintf(context->output, " = ");
                generateExpression(context, varDecl->initializer);
            }
        }
        else
        {
            generateExpression(context, node->initializer);
        }
    }
    fprintf(context->output, "; ");

    // Condition
    if (node->condition != NULL)
    {
        generateExpression(context, node->condition);
    }
    fprintf(context->output, "; ");

    // Increment
    if (node->increment != NULL)
    {
        generateExpression(context, node->increment);
    }
    fprintf(context->output, ") ");

    generateStatement(context, node->body);
}

// Generate code for a return statement
static void generateReturnStatement(CodeGenContext *context, AstReturn *node)
{
    emitIndentation(context);
    fprintf(context->output, "return");

    if (node->value != NULL)
    {
        fprintf(context->output, " ");
        generateExpression(context, node->value);
    }

    fprintf(context->output, ";\n");
}

// Generate code for an expression statement
static void generateExpressionStatement(CodeGenContext *context, AstExpressionStmt *node)
{
    emitIndentation(context);
    generateExpression(context, node->expression);
    fprintf(context->output, ";\n");
}

// Generate code for an expression
static void generateExpression(CodeGenContext *context, AstNode *node)
{
    switch (node->type)
    {
    case AST_BINARY:
        generateBinary(context, (AstBinary *)node);
        break;
    case AST_UNARY:
        generateUnary(context, (AstUnary *)node);
        break;
    case AST_LITERAL:
        generateLiteral(context, (AstLiteral *)node);
        break;
    case AST_VARIABLE:
        generateVariable(context, (AstVariable *)node);
        break;
    case AST_ASSIGNMENT:
        generateAssignment(context, (AstAssignment *)node);
        break;
    case AST_CALL:
        generateCall(context, (AstCall *)node);
        break;
    default:
        fprintf(stderr, "Unknown expression type in code generation.\n");
        break;
    }
}

// Generate code for a binary expression
static void generateBinary(CodeGenContext *context, AstBinary *node)
{
    fprintf(context->output, "(");
    generateExpression(context, node->left);

    // Output the operator
    switch (node->operator)
    {
    case TOKEN_PLUS:
        fprintf(context->output, " + ");
        break;
    case TOKEN_MINUS:
        fprintf(context->output, " - ");
        break;
    case TOKEN_MULTIPLY:
        fprintf(context->output, " * ");
        break;
    case TOKEN_DIVIDE:
        fprintf(context->output, " / ");
        break;
    case TOKEN_MODULO:
        fprintf(context->output, " %% ");
        break;
    case TOKEN_EQUALS:
        fprintf(context->output, " == ");
        break;
    case TOKEN_NOT_EQUALS:
        fprintf(context->output, " != ");
        break;
    case TOKEN_LESS:
        fprintf(context->output, " < ");
        break;
    case TOKEN_GREATER:
        fprintf(context->output, " > ");
        break;
    case TOKEN_LESS_EQ:
        fprintf(context->output, " <= ");
        break;
    case TOKEN_GREATER_EQ:
        fprintf(context->output, " >= ");
        break;
    case TOKEN_AND:
        fprintf(context->output, " && ");
        break;
    case TOKEN_OR:
        fprintf(context->output, " || ");
        break;
    default:
        fprintf(stderr, "Unknown binary operator in code generation.\n");
        break;
    }

    generateExpression(context, node->right);
    fprintf(context->output, ")");
}

// Generate code for a unary expression
static void generateUnary(CodeGenContext *context, AstUnary *node)
{
    // Output the operator
    switch (node->operator)
    {
    case TOKEN_MINUS:
        fprintf(context->output, "(-");
        break;
    case TOKEN_NOT:
        fprintf(context->output, "!");
        break;
    default:
        fprintf(stderr, "Unknown unary operator in code generation.\n");
        break;
    }

    generateExpression(context, node->right);

    if (node->operator== TOKEN_MINUS)
    {
        fprintf(context->output, ")");
    }
}

// Generate code for a literal
static void generateLiteral(CodeGenContext *context, AstLiteral *node)
{
    // Output the literal based on its type
    switch (node->value.type)
    {
    case TOKEN_NUMBER:
        fprintf(context->output, "%.*s", node->value.length, node->value.start);
        break;
    case TOKEN_STRING:
        fprintf(context->output, "\"%.*s\"", node->value.length - 2, node->value.start + 1);
        break;
    default:
        fprintf(stderr, "Unknown literal type in code generation.\n");
        break;
    }
}

// Generate code for a variable reference
static void generateVariable(CodeGenContext *context, AstVariable *node)
{
    fprintf(context->output, "%.*s", node->name.length, node->name.start);
}

// Generate code for an assignment
static void generateAssignment(CodeGenContext *context, AstAssignment *node)
{
    fprintf(context->output, "%.*s = ", node->name.length, node->name.start);
    generateExpression(context, node->value);
}

// Generate code for a function call
static void generateCall(CodeGenContext *context, AstCall *node)
{
    // Map Hindi standard library names to C functions
    if (node->name.length == 4 && strncmp(node->name.start, "लिखो", 4) == 0)
    {
        fprintf(context->output, "printf");
    }
    else if (node->name.length == 3 && strncmp(node->name.start, "पढ़ो", 3) == 0)
    {
        fprintf(context->output, "scanf");
    }
    else
    {
        fprintf(context->output, "%.*s", node->name.length, node->name.start);
    }

    fprintf(context->output, "(");

    // Output the arguments
    for (int i = 0; i < node->argCount; i++)
    {
        if (i > 0)
        {
            fprintf(context->output, ", ");
        }

        generateExpression(context, node->arguments[i]);
    }

    fprintf(context->output, ")");
}