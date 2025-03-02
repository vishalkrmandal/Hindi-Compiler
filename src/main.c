/* src/main.c */
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/ast.h"
#include "../include/semantic.h"
#include "../include/codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Read the entire file into a string
static char *readFile(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "Error: Could not open file '%s'.\n", path);
        return NULL;
    }

    // Get file size
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    // Allocate buffer
    char *buffer = (char *)malloc(fileSize + 1);
    if (buffer == NULL)
    {
        fprintf(stderr, "Error: Not enough memory to read file '%s'.\n", path);
        fclose(file);
        return NULL;
    }

    // Read file
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize)
    {
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

// Get output file path by replacing extension
static char *getOutputPath(const char *inputPath, const char *newExt)
{
    // Get the base name (without extension)
    char *baseName = strdup(inputPath);
    char *dot = strrchr(baseName, '.');
    if (dot != NULL)
    {
        *dot = '\0';
    }

    // Create the new path
    size_t baseLen = strlen(baseName);
    size_t extLen = strlen(newExt);
    char *outputPath = (char *)malloc(baseLen + extLen + 1);

    strcpy(outputPath, baseName);
    strcat(outputPath, newExt);

    free(baseName);
    return outputPath;
}

// Print usage information
static void printUsage(const char *programName)
{
    printf("Usage: %s <input-file> [options]\n", programName);
    printf("Options:\n");
    printf("  -o <output-file>   Specify output file (default: input-file.c)\n");
    printf("  -t                 Tokenize only (output tokens to stdout)\n");
    printf("  -p                 Parse only (no code generation)\n");
    printf("  -h                 Display this help message\n");
}

int main(int argc, char *argv[])
{
    // Check command-line arguments
    if (argc < 2)
    {
        printUsage(argv[0]);
        return 1;
    }

    // Parse command-line options
    char *inputPath = NULL;
    char *outputPath = NULL;
    bool tokenizeOnly = false;
    bool parseOnly = false;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-o") == 0)
        {
            if (i + 1 < argc)
            {
                outputPath = argv[++i];
            }
            else
            {
                fprintf(stderr, "Error: -o option requires an argument.\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "-t") == 0)
        {
            tokenizeOnly = true;
        }
        else if (strcmp(argv[i], "-p") == 0)
        {
            parseOnly = true;
        }
        else if (strcmp(argv[i], "-h") == 0)
        {
            printUsage(argv[0]);
            return 0;
        }
        else if (inputPath == NULL)
        {
            inputPath = argv[i];
        }
        else
        {
            fprintf(stderr, "Error: Unexpected argument '%s'.\n", argv[i]);
            return 1;
        }
    }

    if (inputPath == NULL)
    {
        fprintf(stderr, "Error: No input file specified.\n");
        return 1;
    }

    // Read the input file
    char *source = readFile(inputPath);
    if (source == NULL)
    {
        return 1;
    }

    // Set default output path if not specified
    if (outputPath == NULL)
    {
        outputPath = getOutputPath(inputPath, ".c");
    }

    // Initialize the lexer
    Lexer lexer;
    initLexer(&lexer, source);

    // Tokenize mode
    if (tokenizeOnly)
    {
        Token token;
        do
        {
            token = scanToken(&lexer);
            printf("Token: %s, Line: %d, Column: %d, Text: '%.*s'\n",
                   getTokenName(token.type), token.line, token.column,
                   token.length, token.start);
        } while (token.type != TOKEN_EOF);

        free(source);
        return 0;
    }

    // Initialize the parser
    Parser parser;
    initParser(&parser, &lexer);

    // Parse the source code
    AstProgram *program = parse(&parser);

    if (parser.hadError)
    {
        fprintf(stderr, "Error: Parsing failed.\n");
        free(source);
        return 1;
    }

    if (parseOnly)
    {
        printf("Parsing successful!\n");
        freeAst((AstNode *)program);
        free(source);
        return 0;
    }

    // Semantic analysis
    SymbolTable symbolTable;
    SemanticContext semanticContext;
    initSemanticAnalyzer(&semanticContext, &symbolTable);

    bool semanticSuccess = analyzeProgram(&semanticContext, &symbolTable, program);

    if (!semanticSuccess)
    {
        fprintf(stderr, "Error: Semantic analysis failed with %d errors.\n",
                semanticContext.errorCount);
        freeAst((AstNode *)program);
        freeSymbolTable(&symbolTable);
        free(source);
        return 1;
    }

    // Code generation
    FILE *outputFile = fopen(outputPath, "w");
    if (outputFile == NULL)
    {
        fprintf(stderr, "Error: Could not open output file '%s'.\n", outputPath);
        freeAst((AstNode *)program);
        freeSymbolTable(&symbolTable);
        free(source);
        return 1;
    }

    CodeGenContext codeGenContext;
    initCodeGen(&codeGenContext, outputFile);

    generateCode(&codeGenContext, program);

    printf("Code generation successful! Output written to '%s'.\n", outputPath);

    fclose(outputFile);
    freeAst((AstNode *)program);
    freeSymbolTable(&symbolTable);
    free(source);
    if (outputPath != argv[2])
    { // If we allocated it
        free(outputPath);
    }

    return 0;
}