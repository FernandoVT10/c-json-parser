#ifndef PARSER_INTERNAL_H
#define PARSER_INTERNAL_H

#include <stdio.h>
#include <stdbool.h>

typedef enum {
    STRING_TKN, NUMBER_TKN,
    FALSE_TKN, TRUE_TKN, NULL_TKN,

    OPEN_BRACE_TKN, CLOSE_BRACE_TKN,
    OPEN_BRACKET_TKN, CLOSE_BRACKET_TKN,

    COLON_TKN, COMMA_TKN,
} TokenType;

typedef struct {
    int start;
    int end;
} TokenCol;

typedef struct {
    TokenType type;
    char *lexeme;

    TokenCol col;
    int line;
} Token;

typedef struct {
    Token *items;
    size_t count;
    size_t capacity;
} Tokens;

typedef struct {
    int start; // cursor start
    int current; // cursor current

    char *buffer;
    int line;

    // these keep track the of the column range that belongs to a token
    struct {
        int start;
        int current;
    } col;

    Tokens tokens;
    bool had_errors;
} Lexer;

typedef struct {
    int line;
    struct {
        int start;
        int end;
    } col;
    char *src;
} ErrorRange;

// Lexer

void lexer_init(Lexer *lexer);
bool lexer_scan(Lexer *lexer);
// bool lexer_had_errors();
// void lexer_cleanup();

// Diagnostics
void print_range_error(const char *msg, ErrorRange err);

#endif // PARSER_INTERNAL_H
