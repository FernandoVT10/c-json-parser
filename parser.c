#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "parser_internal.h"

static JsonObject *object(Parser *parser);
static JsonArray *array(Parser *parser);

// Token manipulation functions
static bool is_at_end(Parser *parser) {
    return parser->current == NULL;
}

// static Token get_token(Parser *parser, int pos) {
//     assert(pos < parser->tokens.count && pos >= 0);
//     return parser->tokens.items[pos];
// }

static Token *peek(Parser *parser) {
    return parser->current;
}

static Token *prev(Parser *parser) {
    return parser->current->prev;
}

static Token *next(Parser *parser) {
    Token *t = parser->current;
    parser->current = parser->current->next;
    return t;
}

// returns and advances if the tokens types match
static bool match(Parser *parser, TokenType type) {
    if(is_at_end(parser)) return false;

    if(peek(parser)->type == type) {
        next(parser);
        return true;
    }

    return false;
}

static bool is_next(Parser *parser, TokenType type) {
    if(is_at_end(parser)) return false;
    return peek(parser)->type == type;
}

static void rewind_tkn(Parser *parser) {
    if(parser->current->prev != NULL) {
        parser->current = parser->current->prev;
    }
}

static void synchronize(Parser *parser) {
    while(!is_at_end(parser)) {
        Token *t = peek(parser);
        switch(t->type) {
            case STRING_TKN:
            case CLOSE_BRACE_TKN:
            case CLOSE_BRACKET_TKN:
                return;
            default:
                next(parser);
        }
    }
}

// Error functions

// prints error marking the next token error as the cause
// if there's no next token, a specific error is created
static void error_at_next(Parser *parser, const char *msg) {
    if(is_at_end(parser)) {
        Token *t = prev(parser);
        print_range_error(msg, (ErrorSrc) {
            .line = t->line,
            .col = {t->col.end, t->col.end},
            .src = parser->buffer,
            .eof = true,
        });
    } else {
        Token *t = peek(parser);
        print_range_error(msg, (ErrorSrc) {
            .line = t->line,
            .col = {t->col.start, t->col.end},
            .src = parser->buffer,
        });
    }
    parser->had_errors = true;
}

static void error_msg(Parser *parser, const char *msg) {
    print_message_error(msg);
    parser->had_errors = true;
}


// Parsing functions

static bool value(Parser *parser, JsonValue *value) {
    Token *t = next(parser);

    switch(t->type) {
        case STRING_TKN:
            value->type = JSON_STRING;
            value->ptr = strdup(t->lexeme);
            break;
        case NUMBER_TKN:
            value->type = JSON_NUMBER;
            value->literal = strtod(t->lexeme, NULL);
            break;
        case FALSE_TKN:
            value->type = JSON_BOOL;
            value->literal = 0;
            break;
        case TRUE_TKN:
            value->type = JSON_BOOL;
            value->literal = 1;
            break;
        case NULL_TKN:
            value->type = JSON_NULL;
            break;
        case OPEN_BRACKET_TKN:
            value->type = JSON_ARRAY;
            value->ptr = array(parser);
            break;
        case OPEN_BRACE_TKN:
            value->type = JSON_OBJECT;
            value->ptr = object(parser);
            break;
        default:
            rewind_tkn(parser);
            error_at_next(parser, "Expected value");
            return false;
    }

    return true;
}

static JsonArray *array(Parser *parser) {
    JsonArray *arr = json_array_new();

    bool first = true;
    while(!is_at_end(parser) && !is_next(parser, CLOSE_BRACKET_TKN)) {
        JsonValue val = {0};

        if(first) {
            first = false;
        } else {
            if(!match(parser, COMMA_TKN)) {
                error_at_next(parser, "Expected \",\"");
                synchronize(parser);
                break;
            }
        }

        if(!value(parser, &val)) {
            synchronize(parser);
            break;
        }

        json_array_push(arr, val);
    }

    if(!match(parser, CLOSE_BRACKET_TKN)) {
        error_at_next(parser, "Expected \"]\"");
    }

    return arr;
}

static bool object_item(Parser *parser, JsonObject *obj) {
    if(!is_next(parser, STRING_TKN)) {
        error_at_next(parser, "Expected key");
        return false;
    }

    Token *key = next(parser);

    if(!match(parser, COLON_TKN)) {
        error_at_next(parser, "Expected \":\"");
        return false;
    }

    JsonValue val = {0};
    if(value(parser, &val)) {
        json_object_set(obj, key->lexeme, val);
        return true;
    }

    return false;
}

static JsonObject *object(Parser *parser) {
    JsonObject *obj = json_object_new();

    bool first = true;
    while(!is_at_end(parser) && !is_next(parser, CLOSE_BRACE_TKN)) {
        if(first) {
            first = false;
        } else {
            if(!match(parser, COMMA_TKN)) {
                error_at_next(parser, "Expected \",\"");
                synchronize(parser);
                break;
            }
        }

        if(!object_item(parser, obj)) {
            synchronize(parser);
            break;
        }
    }

    if(!match(parser, CLOSE_BRACE_TKN)) {
        error_at_next(parser, "Expected \"}\"");
        json_object_free(obj);
        return NULL;
    }

    return obj;
}

JsonObject *parser_parse_tokens(Parser *parser) {
    if(parser->tokens.count == 0) {
        error_msg(parser, "Expected \"{\" at the start of the file");
        return NULL;
    }

    parser->current = parser->tokens.head;

    if(!match(parser, OPEN_BRACE_TKN)) {
        error_at_next(parser, "Expected \"{\"");
        return NULL;
    }

    return object(parser);
}
