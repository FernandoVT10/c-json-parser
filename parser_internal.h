#ifndef PARSER_INTERNAL_H
#define PARSER_INTERNAL_H

#include <stdio.h>
#include <stdbool.h>

#include "arena.h"
#include "json_types.h"

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

typedef struct Token Token;

struct Token {
    TokenType type;
    char *lexeme;

    TokenCol col;
    int line;
    Token *prev;
    Token *next;
};

typedef struct {
    Token *head;
    Token *tail;
    size_t count;
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
    Arena arena;
    bool had_errors;
} Lexer;

typedef struct {
    int line;
    struct {
        int start;
        int end;
    } col;
    char *src;
    bool eof; // prints "end of the file" instead of line and column if true
} ErrorSrc;

typedef struct {
    Token* current; // current token
    Tokens tokens;
    char *buffer;
    bool had_errors;
} Parser;

// Lexer
void lexer_init(Lexer *lexer);
bool lexer_scan(Lexer *lexer);
void lexer_free(Lexer *lexer);

// Diagnostics
void print_range_error(const char *msg, ErrorSrc err);
void print_message_error(const char *msg);

// Parser
JsonObject *parser_parse_tokens(Parser *parser);

#endif // PARSER_INTERNAL_H
