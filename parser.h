#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "json_types.h"

typedef struct {
    Tokens tokens;
    size_t cursor;

    bool had_errors;
    const char *buffer;
} Parser;

JsonObject *json_parse(char *buffer);

#endif
