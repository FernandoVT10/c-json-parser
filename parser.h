#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "json_types.h"

typedef struct {
    Tokens tokens;
    size_t cursor;
} Parser;

JsonObject *json_parse(Tokens tokens);

#endif
