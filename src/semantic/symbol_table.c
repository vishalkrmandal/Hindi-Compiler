/* src/semantic/symbol_table.c */
#include "../../include/semantic.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Initialize the symbol table
void initSymbolTable(SymbolTable *table)
{
    table->first = NULL;
    table->scopeDepth = 0;
}

// Create a new symbol
static Symbol *createSymbol(const char *name, SymbolType type, int scopeDepth)
{
    Symbol *symbol = (Symbol *)malloc(sizeof(Symbol));

    // Copy the name
    size_t nameLength = strlen(name);
    symbol->name = (char *)malloc(nameLength + 1);
    strcpy(symbol->name, name);

    symbol->type = type;
    symbol->dataType = TOKEN_VOID; // Default
    symbol->paramCount = 0;
    symbol->paramTypes = NULL;
    symbol->scopeDepth = scopeDepth;
    symbol->next = NULL;

    return symbol;
}

// Define a variable in the symbol table
Symbol *defineVariable(SymbolTable *table, const char *name, TokenType dataType, int line, int column)
{
    // Check for redefinition in the current scope
    Symbol *current = table->first;
    while (current != NULL)
    {
        if (current->scopeDepth == table->scopeDepth &&
            strcmp(current->name, name) == 0)
        {
            fprintf(stderr, "Line %d, Column %d: Error: Variable '%s' already defined in this scope.\n",
                    line, column, name);
            return NULL;
        }
        current = current->next;
    }

    // Create the new symbol
    Symbol *symbol = createSymbol(name, SYMBOL_VARIABLE, table->scopeDepth);
    symbol->dataType = dataType;

    // Add to the start of the linked list
    symbol->next = table->first;
    table->first = symbol;

    return symbol;
}

// Define a function in the symbol table
Symbol *defineFunction(SymbolTable *table, const char *name, TokenType returnType,
                       int paramCount, TokenType *paramTypes, int line, int column)
{
    // Check for redefinition at global scope
    Symbol *current = table->first;
    while (current != NULL)
    {
        if (current->scopeDepth == 0 && strcmp(current->name, name) == 0)
        {
            fprintf(stderr, "Line %d, Column %d: Error: Function '%s' already defined.\n",
                    line, column, name);
            return NULL;
        }
        current = current->next;
    }

    // Create the new symbol
    Symbol *symbol = createSymbol(name, SYMBOL_FUNCTION, 0); // Functions always at global scope
    symbol->dataType = returnType;
    symbol->paramCount = paramCount;

    // Copy parameter types
    if (paramCount > 0)
    {
        symbol->paramTypes = (TokenType *)malloc(sizeof(TokenType) * paramCount);
        memcpy(symbol->paramTypes, paramTypes, sizeof(TokenType) * paramCount);
    }

    // Add to the start of the linked list
    symbol->next = table->first;
    table->first = symbol;

    return symbol;
}

// Resolve a symbol from the symbol table
Symbol *resolveSymbol(SymbolTable *table, const char *name)
{
    Symbol *current = table->first;

    while (current != NULL)
    {
        if (strcmp(current->name, name) == 0)
        {
            return current;
        }
        current = current->next;
    }

    return NULL; // Not found
}

// Begin a new scope
void beginScope(SymbolTable *table)
{
    table->scopeDepth++;
}

// End the current scope
void endScope(SymbolTable *table)
{
    // Remove symbols from the current scope
    Symbol *current = table->first;
    Symbol *previous = NULL;

    while (current != NULL)
    {
        if (current->scopeDepth == table->scopeDepth)
        {
            // Remove this symbol
            Symbol *toDelete = current;

            if (previous == NULL)
            {
                // It's the first element
                table->first = current->next;
                current = table->first;
            }
            else
            {
                // It's not the first element
                previous->next = current->next;
                current = previous->next;
            }

            // Free the symbol
            free(toDelete->name);
            if (toDelete->paramTypes != NULL)
            {
                free(toDelete->paramTypes);
            }
            free(toDelete);
        }
        else
        {
            // Keep this symbol
            previous = current;
            current = current->next;
        }
    }

    table->scopeDepth--;
}

// Free the entire symbol table
void freeSymbolTable(SymbolTable *table)
{
    Symbol *current = table->first;

    while (current != NULL)
    {
        Symbol *next = current->next;

        free(current->name);
        if (current->paramTypes != NULL)
        {
            free(current->paramTypes);
        }
        free(current);

        current = next;
    }

    table->first = NULL;
    table->scopeDepth = 0;
}

// Report semantic errors
void semanticError(SemanticContext *context, int line, int column, const char *message)
{
    fprintf(stderr, "Line %d, Column %d: Error: %s\n", line, column, message);
    context->errorCount++;
}