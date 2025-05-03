#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

JsonArray *parser_array();
JsonObject *parser_object(bool test_first_brace);

// FORMAL GRAMMAR
// Object => "{" (ObjectItem ("," ObjectItem)*)? "}"
// ObjectItem => Key ":" Value
// Key => '"' [0-9A-Za-z]+ '"'
// Value => "false" | "true" | "null" | Number | String | Array | Object
// Number => [0-9]+
// String => '"' [0-9A-Za-z]* '"'
// Array => "[" (Value ("," Value)*)? "]"

Parser parser = {0};

bool parser_is_at_end() {
    return parser.cursor >= parser.tokens.count;
}

Token *parser_get_tkn(size_t pos) {
    if(pos >= parser.tokens.count) {
        return NULL;
    }

    return &parser.tokens.items[pos];
}

Token *parser_consume_tkn() {
    return parser_get_tkn(parser.cursor++);
}

Token *parser_peek_tkn() {
    return parser_get_tkn(parser.cursor);
}

Token *parser_prev_tkn() {
    if(parser.cursor > 0) {
        return parser_get_tkn(parser.cursor - 1);
    }

    return NULL;
}

bool parser_match_tkn(TokenType type) {
    Token *token = parser_peek_tkn();
    if(token != NULL && token->type == type) {
        parser_consume_tkn();
        return true;
    }

    return false;
}
bool parser_is_next_tkn(TokenType type) {
    Token *t = parser_peek_tkn();
    return t != NULL && t->type == type;
}

bool parser_value(JsonValue *value) {
    Token *t = parser_consume_tkn();

    switch(t->type) {
        case STRING_TKN:
            value->type = JSON_STRING;
            // TODO: is this ok?
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
            value->ptr = parser_array();
            break;
        case OPEN_BRACE_TKN:
            value->type = JSON_OBJECT;
            value->ptr = parser_object(false);
            break;
        default:
            printf("[ERROR] Expected value\n");
            return false;
    }

    return true;
}

JsonArray *parser_array() {
    JsonArray *arr = json_array_new();

    bool first = true;
    while(!parser_is_at_end() && !parser_is_next_tkn(CLOSE_BRACKET_TKN)) {
        JsonValue value = {0};

        if(first) {
            if(!parser_value(&value)) break;
            first = false;
        } else {
            if(!parser_match_tkn(COMMA_TKN)) {
                printf("[ERROR] Expected \",\"\n");
                break;
            }

            if(!parser_value(&value)) break;
        }

        json_array_push(arr, value);
    }

    if(!parser_match_tkn(CLOSE_BRACKET_TKN)) {
        printf("[ERROR] Expected \"]\"\n");
    }

    return arr;
}

void parser_object_item(JsonObject *obj) {
    if(!parser_is_next_tkn(STRING_TKN)) {
        printf("[ERROR] Expected key\n");
        return;
    }

    Token *key = parser_consume_tkn();

    if(!parser_match_tkn(COLON_TKN)) {
        printf("[ERROR] Expected \":\"\n");
        return;
    }

    JsonValue value = {0};
    if(parser_value(&value)) {
        json_object_set(obj, key->lexeme, value);
    }
}

JsonObject *parser_object(bool test_first_brace) {
    JsonObject *obj = json_object_new();

    if(test_first_brace && !parser_match_tkn(OPEN_BRACE_TKN)) {
        printf("[ERROR] Expected \"{\"\n");
    }

    bool first = true;
    while(!parser_is_next_tkn(CLOSE_BRACE_TKN) && !parser_is_at_end()) {
        if(first) {
            parser_object_item(obj);
            first = false;
        } else {
            if(!parser_match_tkn(COMMA_TKN)) {
                printf("[ERROR] Expected \",\"\n");
                break;
            }

            parser_object_item(obj);
        }
    }

    if(!parser_match_tkn(CLOSE_BRACE_TKN)) {
        printf("[ERROR] Expected \"}\"\n");
    }

    return obj;
}

JsonObject *json_parse(Tokens tokens) {
    parser.tokens = tokens;
    parser.cursor = 0;

    JsonObject *obj = parser_object(true);

    return obj;
}
