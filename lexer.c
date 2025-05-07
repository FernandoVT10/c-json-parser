#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "cTooling.h"
#include "lexer.h"
#include "utils.h"

Lexer lexer = {0};

void lexer_init(char *buffer) {
    bzero(&lexer, sizeof(Lexer));
    lexer.line = 1;
    lexer.buffer = buffer;
}

bool lexer_is_at_end() {
    return lexer.cur_current >= strlen(lexer.buffer);
}

char lexer_advance() {
    lexer.col_current++;
    return lexer.buffer[lexer.cur_current++];
}

char lexer_peek() {
    return lexer.buffer[lexer.cur_current];
}

bool lexer_match(char c) {
    if(lexer_peek() == c) {
        lexer_advance();
        return true;
    }

    return false;
}

void lexer_add_tkn(TokenType type, char *lexeme) {
    Token token = {
        .type = type,
        .lexeme = lexeme,
        .start_col = lexer.col_start,
        .end_col = lexer.col_current,
        .line = lexer.line,
    };
    lexer.col_start = lexer.col_current;
    lexer.cur_start = lexer.cur_current;
    da_append(&lexer.tokens, token);
}

void lexer_error(const char *message, int start_col, int end_col, int line) {
    syntax_error(lexer.buffer, message, start_col, end_col, line);
    lexer.had_errors = true;
}

void lexer_string() {
    while(!lexer_is_at_end() && lexer_peek() != '"' && lexer_peek() != '\n') {
        lexer_advance();
    }

    if(!lexer_match('"')) {
        lexer_error("Expected terminating \"", lexer.col_start, lexer.col_current, lexer.line);
        return;
    }

    char *str = strndup(
        // +1 to remove the preceding "
        lexer.buffer + lexer.cur_start + 1,
        // -2 to remove the ending " and the added char above
        lexer.cur_current - lexer.cur_start - 2
    );
    lexer_add_tkn(STRING_TKN, str);
}

void lexer_number(char c) {
    if(c == '-' && !isdigit(lexer_peek())) {
        lexer_error("Expected digit after \"-\"", lexer.col_current - 1, lexer.col_current, lexer.line);
        return;
    }

    while(!lexer_is_at_end() && isdigit(lexer_peek())) {
        lexer_advance();
    }

    if(lexer_match('.')) {
        if(!isdigit(lexer_peek())) {
            lexer_error("Expected digit after \".\"", lexer.col_current - 1, lexer.col_current, lexer.line);
            return;
        }

        while(!lexer_is_at_end() && isdigit(lexer_peek())) {
            lexer_advance();
        }
    }

    if(lexer_match('e')) {
        if(!isdigit(lexer_peek())) {
            lexer_error("Expected digit after \"e\"", lexer.col_current - 1, lexer.col_current, lexer.line);
            return;
        }

        while(!lexer_is_at_end() && isdigit(lexer_peek())) {
            lexer_advance();
        }
    }

    char *number = strndup(lexer.buffer + lexer.cur_start, lexer.cur_current - lexer.cur_start);
    lexer_add_tkn(NUMBER_TKN, number);
}

void lexer_keyword(char c) {
    while(!lexer_is_at_end() && isalpha(lexer_peek())) {
        lexer_advance();
    }

    char *keyword = strndup(lexer.buffer + lexer.cur_start, lexer.cur_current - lexer.cur_start);

    if(strcmp(keyword, "false") == 0) {
        lexer_add_tkn(FALSE_TKN, NULL);
    } else if(strcmp(keyword, "true") == 0) {
        lexer_add_tkn(TRUE_TKN, NULL);
    } else if(strcmp(keyword, "null") == 0) {
        lexer_add_tkn(NULL_TKN, NULL);
    } else {
        const char *message = text_format("Unknown keyword \"%s\"", keyword);
        lexer_error(message, lexer.col_start, lexer.col_current, lexer.line);
    }

    free(keyword);
}

Tokens lexer_scan() {
    while(!lexer_is_at_end()) {
        char c = lexer_advance();

        switch(c) {
            case '"': lexer_string(); break;
            case '{': lexer_add_tkn(OPEN_BRACE_TKN, NULL); break;
            case '}': lexer_add_tkn(CLOSE_BRACE_TKN, NULL); break;
            case ':': lexer_add_tkn(COLON_TKN, NULL); break;
            case '[': lexer_add_tkn(OPEN_BRACKET_TKN, NULL); break;
            case ']': lexer_add_tkn(CLOSE_BRACKET_TKN, NULL); break;
            case ',': lexer_add_tkn(COMMA_TKN, NULL); break;
            case '\n':
                // resets col_current and col_start when a new line is found
                // to be able to tokenize the next chunk correctly
                lexer.col_start = 0;
                lexer.col_current = 0;
                lexer.cur_start = lexer.cur_current;
                lexer.line++;
                break;
            case '\t':
            case ' ':
                lexer.col_start = lexer.col_current;
                lexer.cur_start = lexer.cur_current;
                break;
            default: {
                if(isalpha(c)) {
                    lexer_keyword(c);
                } else if(isdigit(c) || c == '-') {
                    lexer_number(c);
                } else {
                    const char *msg = text_format("Unexpected character \"%c\"", c);
                    lexer_error(msg, lexer.col_current - 1, lexer.col_current, lexer.line);
                }
            }
        }
    }

    return lexer.tokens;
}

bool lexer_had_errors() {
    return lexer.had_errors;
}

void lexer_cleanup() {
    for(size_t i = 0; i < lexer.tokens.count; i++) {
        Token token = lexer.tokens.items[i];
        if(token.type == STRING_TKN || token.type == NUMBER_TKN) {
            free(token.lexeme);
        }
    }

    da_free(&lexer.tokens);
}
