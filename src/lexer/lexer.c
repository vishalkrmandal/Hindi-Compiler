/* src/lexer/lexer.c */
#include "../../include/lexer.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// Keyword mapping structure
typedef struct
{
    const char *keyword;
    TokenType token;
} KeywordMap;

// Hindi keywords mapping table
static const KeywordMap hindiKeywords[] = {
    {"पूर्णांक", TOKEN_INT},
    {"दशमलव", TOKEN_FLOAT},
    {"वर्ण", TOKEN_CHAR},
    {"शून्य", TOKEN_VOID},
    {"अगर", TOKEN_IF},
    {"वरना", TOKEN_ELSE},
    {"दौर", TOKEN_FOR},
    {"जबतक", TOKEN_WHILE},
    {"करो", TOKEN_DO},
    {"रुको", TOKEN_BREAK},
    {"जारी", TOKEN_CONTINUE},
    {"वापस", TOKEN_RETURN},
    {NULL, 0} // End sentinel
};

void initLexer(Lexer *lexer, const char *source)
{
    lexer->start = source;
    lexer->current = source;
    lexer->line = 1;
    lexer->column = 1;
}

// Helper functions
static bool isAtEnd(Lexer *lexer)
{
    return *lexer->current == '\0';
}

static char advance(Lexer *lexer)
{
    lexer->column++;
    return *(lexer->current++);
}

static char peek(Lexer *lexer)
{
    return *lexer->current;
}

static char peekNext(Lexer *lexer)
{
    if (isAtEnd(lexer))
        return '\0';
    return lexer->current[1];
}

static bool match(Lexer *lexer, char expected)
{
    if (isAtEnd(lexer))
        return false;
    if (*lexer->current != expected)
        return false;

    lexer->current++;
    lexer->column++;
    return true;
}

static Token makeToken(Lexer *lexer, TokenType type)
{
    Token token;
    token.type = type;
    token.start = lexer->start;
    token.length = (int)(lexer->current - lexer->start);
    token.line = lexer->line;
    token.column = lexer->column - token.length;
    return token;
}

static Token errorToken(Lexer *lexer, const char *message)
{
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = lexer->line;
    token.column = lexer->column;
    return token;
}

// Skip whitespace and comments
static void skipWhitespace(Lexer *lexer)
{
    for (;;)
    {
        char c = peek(lexer);
        switch (c)
        {
        case ' ':
        case '\r':
        case '\t':
            advance(lexer);
            break;
        case '\n':
            lexer->line++;
            lexer->column = 1;
            advance(lexer);
            break;
        case '/':
            if (peekNext(lexer) == '/')
            {
                // Comment until end of line
                while (peek(lexer) != '\n' && !isAtEnd(lexer))
                    advance(lexer);
            }
            else
            {
                return;
            }
            break;
        default:
            return;
        }
    }
}

// Check if character is a digit
static bool isDigit(char c)
{
    return c >= '0' && c <= '9';
}

// Simplified function to check if a character is a Hindi character
// In a real implementation, you would need proper UTF-8 handling
static bool isHindiChar(const char *str)
{
    // Check if we have a valid UTF-8 3-byte sequence for Hindi
    // Hindi Unicode range is approximately 0x0900 to 0x097F
    if ((unsigned char)str[0] == 0xE0 &&
        ((unsigned char)str[1] == 0xA4 || (unsigned char)str[1] == 0xA5))
    {
        return true;
    }
    return false;
}

// Check if character can start an identifier
static bool isIdentifierStart(char c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_' ||
           (unsigned char)c >= 0xE0; // Simplified check for first byte of Hindi character
}

static bool isIdentifierPart(char c)
{
    return isIdentifierStart(c) || isDigit(c);
}

// Parse identifier and check if it's a keyword
static Token identifier(Lexer *lexer)
{
    while (isIdentifierPart(peek(lexer)))
    {
        advance(lexer);
    }

    // Check if the identifier is a keyword
    for (int i = 0; hindiKeywords[i].keyword != NULL; i++)
    {
        const char *keyword = hindiKeywords[i].keyword;
        size_t length = strlen(keyword);

        if (lexer->current - lexer->start == length &&
            memcmp(lexer->start, keyword, length) == 0)
        {
            return makeToken(lexer, hindiKeywords[i].token);
        }
    }

    return makeToken(lexer, TOKEN_IDENTIFIER);
}

// Parse number literal
static Token number(Lexer *lexer)
{
    while (isDigit(peek(lexer)))
    {
        advance(lexer);
    }

    // Look for a fractional part
    if (peek(lexer) == '.' && isDigit(peekNext(lexer)))
    {
        // Consume the '.'
        advance(lexer);

        while (isDigit(peek(lexer)))
        {
            advance(lexer);
        }

        Token token = makeToken(lexer, TOKEN_NUMBER);
        token.value.float_value = (float)strtod(lexer->start, NULL);
        return token;
    }

    Token token = makeToken(lexer, TOKEN_NUMBER);
    token.value.int_value = (int)strtol(lexer->start, NULL, 10);
    return token;
}

// Parse string literal
static Token string(Lexer *lexer)
{
    while (peek(lexer) != '"' && !isAtEnd(lexer))
    {
        if (peek(lexer) == '\n')
        {
            lexer->line++;
            lexer->column = 1;
        }
        advance(lexer);
    }

    if (isAtEnd(lexer))
    {
        return errorToken(lexer, "Unterminated string.");
    }

    // Consume the closing quote
    advance(lexer);

    // Capture the string value (without the quotes)
    Token token = makeToken(lexer, TOKEN_STRING);
    int length = token.length - 2; // Exclude the quotes
    char *value = (char *)malloc(length + 1);
    memcpy(value, token.start + 1, length);
    value[length] = '\0';
    token.value.string_value = value;

    return token;
}

// Scan the next token
Token scanToken(Lexer *lexer)
{
    skipWhitespace(lexer);

    lexer->start = lexer->current;

    if (isAtEnd(lexer))
        return makeToken(lexer, TOKEN_EOF);

    char c = advance(lexer);

    // Identifiers
    if (isIdentifierStart(c))
        return identifier(lexer);

    // Numbers
    if (isDigit(c))
        return number(lexer);

    // Other tokens
    switch (c)
    {
    // Single character tokens
    case '(':
        return makeToken(lexer, TOKEN_LPAREN);
    case ')':
        return makeToken(lexer, TOKEN_RPAREN);
    case '{':
        return makeToken(lexer, TOKEN_LBRACE);
    case '}':
        return makeToken(lexer, TOKEN_RBRACE);
    case ';':
        return makeToken(lexer, TOKEN_SEMICOLON);
    case ',':
        return makeToken(lexer, TOKEN_COMMA);

    // One or two character tokens
    case '+':
        return makeToken(lexer, TOKEN_PLUS);
    case '-':
        return makeToken(lexer, TOKEN_MINUS);
    case '*':
        return makeToken(lexer, TOKEN_MULTIPLY);
    case '/':
        return makeToken(lexer, TOKEN_DIVIDE);
    case '%':
        return makeToken(lexer, TOKEN_MODULO);

    case '=':
        return makeToken(lexer, match(lexer, '=') ? TOKEN_EQUALS : TOKEN_ASSIGN);
    case '!':
        return makeToken(lexer, match(lexer, '=') ? TOKEN_NOT_EQUALS : TOKEN_NOT);
    case '<':
        return makeToken(lexer, match(lexer, '=') ? TOKEN_LESS_EQ : TOKEN_LESS);
    case '>':
        return makeToken(lexer, match(lexer, '=') ? TOKEN_GREATER_EQ : TOKEN_GREATER);

    case '&':
        if (match(lexer, '&'))
            return makeToken(lexer, TOKEN_AND);
        break;
    case '|':
        if (match(lexer, '|'))
            return makeToken(lexer, TOKEN_OR);
        break;

    // String literals
    case '"':
        return string(lexer);
    }

    return errorToken(lexer, "Unexpected character.");
}

// Get readable token name for debugging
const char *getTokenName(TokenType type)
{
    switch (type)
    {
    case TOKEN_EOF:
        return "EOF";
    case TOKEN_INT:
        return "INT";
    case TOKEN_FLOAT:
        return "FLOAT";
    case TOKEN_CHAR:
        return "CHAR";
    case TOKEN_VOID:
        return "VOID";
    case TOKEN_IF:
        return "IF";
    case TOKEN_ELSE:
        return "ELSE";
    case TOKEN_FOR:
        return "FOR";
    case TOKEN_WHILE:
        return "WHILE";
    case TOKEN_DO:
        return "DO";
    case TOKEN_BREAK:
        return "BREAK";
    case TOKEN_CONTINUE:
        return "CONTINUE";
    case TOKEN_RETURN:
        return "RETURN";
    case TOKEN_IDENTIFIER:
        return "IDENTIFIER";
    case TOKEN_NUMBER:
        return "NUMBER";
    case TOKEN_STRING:
        return "STRING";
    case TOKEN_PLUS:
        return "PLUS";
    case TOKEN_MINUS:
        return "MINUS";
    case TOKEN_MULTIPLY:
        return "MULTIPLY";
    case TOKEN_DIVIDE:
        return "DIVIDE";
    case TOKEN_MODULO:
        return "MODULO";
    case TOKEN_ASSIGN:
        return "ASSIGN";
    case TOKEN_EQUALS:
        return "EQUALS";
    case TOKEN_NOT_EQUALS:
        return "NOT_EQUALS";
    case TOKEN_GREATER:
        return "GREATER";
    case TOKEN_LESS:
        return "LESS";
    case TOKEN_GREATER_EQ:
        return "GREATER_EQ";
    case TOKEN_LESS_EQ:
        return "LESS_EQ";
    case TOKEN_AND:
        return "AND";
    case TOKEN_OR:
        return "OR";
    case TOKEN_NOT:
        return "NOT";
    case TOKEN_SEMICOLON:
        return "SEMICOLON";
    case TOKEN_COMMA:
        return "COMMA";
    case TOKEN_LPAREN:
        return "LPAREN";
    case TOKEN_RPAREN:
        return "RPAREN";
    case TOKEN_LBRACE:
        return "LBRACE";
    case TOKEN_RBRACE:
        return "RBRACE";
    case TOKEN_ERROR:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}