#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "cTooling.h"
#include "lexer.h"
#include "utils.h"

Lexer lexer = {0};

void lexer_init(char *buffer) {
    lexer.buffer = buffer;
    lexer.line = 1;
    lexer.start_col = 0;
    lexer.cur_col = 0;
}

bool lexer_is_at_end() {
    return lexer.cursor >= strlen(lexer.buffer);
}

char lexer_advance() {
    lexer.cur_col++;
    return lexer.buffer[lexer.cursor++];
}

char lexer_peek() {
    return lexer.buffer[lexer.cursor];
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
        .start_col = lexer.start_col,
        .end_col = lexer.cur_col,
        .line = lexer.line,
    };
    lexer.start_col = lexer.cur_col;
    da_append(&lexer.tokens, token);
}

void lexer_error(const char *message, int start_col, int end_col, int line) {
    syntax_error(lexer.buffer, message, start_col, end_col, line);
    lexer.had_errors = true;
}

void lexer_string() {
    String str = {0};
    while(!lexer_is_at_end() && lexer_peek() != '"' && lexer_peek() != '\n') {
        string_append_chr(&str, lexer_advance());
    }

    if(!lexer_match('"')) {
        lexer_error("Expected terminating \"", lexer.start_col, lexer.cur_col, lexer.line);
        goto end;
    }

    lexer_add_tkn(STRING_TKN, string_dump(&str));
// TODO: is it call when there was no error?
end:
    string_free(&str);
}

void lexer_number(char c) {
    String number = {0};
    string_append_chr(&number, c);

    if(c == '-' && !isdigit(lexer_peek())) {
        lexer_error("Expected digit after \"-\"", lexer.cur_col - 1, lexer.cur_col, lexer.line);
        goto end;
    }

    while(!lexer_is_at_end() && isdigit(lexer_peek())) {
        string_append_chr(&number, lexer_advance());
    }

    if(lexer_match('.')) {
        string_append_chr(&number, '.');

        if(!isdigit(lexer_peek())) {
            lexer_error("Expected digit after \".\"", lexer.cur_col - 1, lexer.cur_col, lexer.line);
            goto end;
        }

        while(!lexer_is_at_end() && isdigit(lexer_peek())) {
            string_append_chr(&number, lexer_advance());
        }
    }

    if(lexer_match('e')) {
        string_append_chr(&number, 'e');

        if(!isdigit(lexer_peek())) {
            lexer_error("Expected digit after \"e\"", lexer.cur_col - 1, lexer.cur_col, lexer.line);
            goto end;
        }

        while(!lexer_is_at_end() && isdigit(lexer_peek())) {
            string_append_chr(&number, lexer_advance());
        }
    }

    lexer_add_tkn(NUMBER_TKN, string_dump(&number));
end:
    string_free(&number);
}

void lexer_keyword(char c) {
    String str = {0};
    string_append_chr(&str, c);
    while(!lexer_is_at_end() && isalpha(lexer_peek())) {
        string_append_chr(&str, lexer_advance());
    }

    if(string_cmp_text(&str, "false") == 0) {
        lexer_add_tkn(FALSE_TKN, NULL);
    } else if(string_cmp_text(&str, "true") == 0) {
        lexer_add_tkn(TRUE_TKN, NULL);
    } else if(string_cmp_text(&str, "null") == 0) {
        lexer_add_tkn(NULL_TKN, NULL);
    } else {
        char *keyword = string_dump(&str);
        const char *message = text_format("Unknown keyword \"%s\"", keyword);
        lexer_error(message, lexer.start_col, lexer.cur_col, lexer.line);
        free(keyword);
    }

    string_free(&str);
}

Tokens lexer_scan() {
    while(!lexer_is_at_end()) {
        char c = lexer_advance();

        switch(c) {
            case '"':
                lexer_string();
                break;
            case '{':
                lexer_add_tkn(OPEN_BRACE_TKN, NULL);
                break;
            case '}':
                lexer_add_tkn(CLOSE_BRACE_TKN, NULL);
                break;
            case ':':
                lexer_add_tkn(COLON_TKN, NULL);
                break;
            case '[':
                lexer_add_tkn(OPEN_BRACKET_TKN, NULL);
                break;
            case ']':
                lexer_add_tkn(CLOSE_BRACKET_TKN, NULL);
                break;
            case ',':
                lexer_add_tkn(COMMA_TKN, NULL);
                break;
            case '\n':
                lexer.line++;
                lexer.cur_col = 0;
                lexer.start_col = 0;
                break;
            case '\t':
            case ' ':
                lexer.start_col = lexer.cur_col;
                continue;
            default: {
                if(isalpha(c)) {
                    lexer_keyword(c);
                } else if(isdigit(c) || c == '-') {
                    lexer_number(c);
                } else {
                    const char *msg = text_format("Unexpected character \"%c\"", c);
                    lexer_error(msg, lexer.cur_col - 1, lexer.cur_col, lexer.line);
                }
            }
        }
    }

    return lexer.tokens;
}

bool lexer_had_errors() {
    return lexer.had_errors;
}
