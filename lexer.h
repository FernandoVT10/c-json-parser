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

    int start_col;
    int end_col;
    int line;
} Token;

typedef struct {
    Token *items;
    size_t count;
    size_t capacity;
} Tokens;

typedef struct {
    int cur_start; // cursor start
    int cur_current; // cursor current

    char *buffer;
    int line;

    // these keep track the start and end column of a token
    int col_start;
    int col_current;

    Tokens tokens;
    bool had_errors;
} Lexer;

void lexer_init(char *buffer);
Tokens lexer_scan();
bool lexer_had_errors();
void lexer_cleanup();

#endif // LEXER_H
