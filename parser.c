#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "error.h"
#include "cTooling.h"

static JsonArray *parse_array();
static JsonObject *parse_object();

Parser parser = {0};

static bool is_at_end() {
    return parser.cursor >= parser.tokens.count;
}

static Token *get_tkn(int pos) {
    if(pos < 0 || pos >= parser.tokens.count) {
        return NULL;
    }

    return &parser.tokens.items[pos];
}

static Token *consume_tkn() {
    return get_tkn(parser.cursor++);
}

static Token *peek_tkn() {
    return get_tkn(parser.cursor);
}

static Token *prev_tkn() {
    return get_tkn(parser.cursor - 1);
}

static bool match_tkn(TokenType type) {
    Token *token = peek_tkn();
    if(token != NULL && token->type == type) {
        consume_tkn();
        return true;
    }

    return false;
}

static bool is_next_tkn(TokenType type) {
    Token *t = peek_tkn();
    return t != NULL && t->type == type;
}

static void rewind_tkn() {
    if(parser.cursor > 0) {
        parser.cursor--;
    }
}

void parser_error(const char *message, Token *token) {
    syntax_error(parser.buffer, message, token->start_col, token->end_col, token->line);
    parser.had_errors = true;
}

static bool parse_value(JsonValue *value) {
    Token *t = consume_tkn();

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
            value->ptr = parse_array();
            break;
        case OPEN_BRACE_TKN:
            value->type = JSON_OBJECT;
            value->ptr = parse_object(false);
            break;
        default:
            rewind_tkn();
            return false;
    }

    return true;
}

static JsonArray *parse_array() {
    JsonArray *arr = json_array_new();

    bool first = true;
    while(!is_at_end() && !is_next_tkn(CLOSE_BRACKET_TKN)) {
        JsonValue value = {0};

        if(first) {
            if(!parse_value(&value)) break;
            first = false;
        } else {
            if(!match_tkn(COMMA_TKN)) {
                printf("[ERROR] Expected \",\"\n");
                break;
            }

            if(!parse_value(&value)) break;
        }

        json_array_push(arr, value);
    }

    if(!match_tkn(CLOSE_BRACKET_TKN)) {
        printf("[ERROR] Expected \"]\"\n");
    }

    return arr;
}

static void parse_object_item(JsonObject *obj) {
    if(!is_next_tkn(STRING_TKN)) {
        parser_error("Expected key", peek_tkn());
        return;
    }

    Token *key = consume_tkn();

    if(!match_tkn(COLON_TKN)) {
        printf("[ERROR] Expected \":\"\n");
        return;
    }

    JsonValue value = {0};
    if(parse_value(&value)) {
        json_object_set(obj, key->lexeme, value);
    } else {
        parser_error("Expected value after \":\"", prev_tkn());
    }
}

static JsonObject *parse_object() {
    JsonObject *obj = json_object_new();

    // if(test_first_brace && !match_tkn(OPEN_BRACE_TKN)) {
    //     Token *prev_tkn = prev_tkn();
    //
    //     if(prev_tkn == NULL) {
    //         syntax_error(parser.buffer, "Expected starting \"{\"", 0, 0, 0);
    //         parser.had_errors = true;
    //     } else {
    //         parser_error("Expected opening \"{\" after \":\"", prev_tkn);
    //     }
    //
    //     return NULL;
    // }

    bool first = true;
    while(!is_next_tkn(CLOSE_BRACE_TKN) && !is_at_end()) {
        if(first) {
            parse_object_item(obj);
            first = false;
        } else {
            if(!match_tkn(COMMA_TKN)) {
                char *abc;

                switch(peek_tkn()->type) {
                    case STRING_TKN: abc = "string"; break;
                    case NUMBER_TKN: abc = "number"; break;
                    case FALSE_TKN: abc = "false"; break;
                    case TRUE_TKN: abc = "true"; break;
                    case NULL_TKN: abc = "null"; break;
                    case OPEN_BRACE_TKN: abc = "\"{\""; break;
                    case CLOSE_BRACE_TKN: abc = "\"}\""; break;
                    case OPEN_BRACKET_TKN: abc = "\"[\""; break;
                    case CLOSE_BRACKET_TKN: abc = "\"]\""; break;
                    case COLON_TKN: abc = "\":\""; break;
                    case COMMA_TKN: abc = "\",\""; break;
                }

                const char *error = text_format("Expected \",\" after value but got %s", abc);
                parser_error(error, peek_tkn());

                while(!is_at_end() && !is_next_tkn(STRING_TKN)) {
                    consume_tkn();
                }
            }

            parse_object_item(obj);
        }
    }

    if(!match_tkn(CLOSE_BRACE_TKN)) {
        if(is_at_end()) {
            parser_error("Expected \"}\" at the end of the file", prev_tkn());
        } else {
            // TODO
            parser_error("Expected \"}\" but got s", prev_tkn());
        }
        return NULL;
    }

    return obj;
}

JsonObject *json_parse(char *buffer) {
    lexer_init(buffer);
    parser.tokens = lexer_scan();
    if(lexer_had_errors()) {
        return NULL;
    }

    parser.cursor = 0;
    parser.buffer = buffer;

    if(!match_tkn(OPEN_BRACE_TKN)) {
        if(is_at_end()) {
            syntax_error(parser.buffer, "Expected \"{\" at the start of the file", 0, 0, 0);
        } else {
            parser_error("Expected \"{\" but got", peek_tkn());
        }
        return NULL;
    }

    return NULL;

    JsonObject *obj = parse_object();

    lexer_cleanup();

    if(parser.had_errors) {
        return NULL;
    }

    return obj;
}
