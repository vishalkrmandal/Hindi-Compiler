/* include/codegen.h */
#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"

// Code generator context
typedef struct
{
    FILE *output;    // Output file for generated code
    int indentLevel; // Current indentation level
} CodeGenContext;

// Initialize the code generator
void initCodeGen(CodeGenContext *context, FILE *output);

// Generate code from AST
void generateCode(CodeGenContext *context, AstProgram *program);

// Helper functions
void emitIndentation(CodeGenContext *context);
void emitLine(CodeGenContext *context, const char *format, ...);

#endif /* CODEGEN_H */