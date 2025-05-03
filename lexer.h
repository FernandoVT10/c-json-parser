#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

typedef enum {
    STRING_TKN, NUMBER_TKN,
    FALSE_TKN, TRUE_TKN, NULL_TKN,

    OPEN_BRACE_TKN, CLOSE_BRACE_TKN,
    OPEN_BRACKET_TKN, CLOSE_BRACKET_TKN,

    COLON_TKN, COMMA_TKN,
} TokenType;

typedef struct {
    TokenType type;
    char *lexeme;
} Token;

typedef struct {
    Token *items;
    size_t count;
    size_t capacity;
} Tokens;

typedef struct {
    size_t cursor;
    char *buffer;
    int line;
    int col;

    Tokens tokens;
    bool had_errors;
} Lexer;

void lexer_init(char *buffer);
Tokens lexer_scan();
bool lexer_had_errors();

#endif // LEXER_H
