#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "cTooling.h"
#include "lexer.h"

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"

Lexer lexer = {0};

typedef enum {
    ANSI_RED,
} AnsiColor;

void print_slice(FILE *stream, const char *text, int start, int end) {
    for(size_t i = start; i < end; i++) {
        putc(text[i], stream);
    }
}

void print_slice_colored(FILE *stream, const char *text, int start, int end, AnsiColor color) {
    switch(color) {
        case ANSI_RED:
            fprintf(stream, ANSI_COLOR_RED);
            print_slice(stream, text, start, end);
            fprintf(stream, ANSI_COLOR_RESET);
        break;
    }
}

int *get_buf_line_pos(int line) {
    static int buf_line[2];
    buf_line[0] = 0;
    buf_line[1] = 0;

    int x = 0;
    while(line > 1 && x < strlen(lexer.buffer)) {
        if(lexer.buffer[x] == '\n') line--;
        buf_line[0]++;
        x++;
    }

    buf_line[1] = buf_line[0];
    while(buf_line[1] < strlen(lexer.buffer) && lexer.buffer[buf_line[1]] != '\n') {
        buf_line[1]++;
    }

    return buf_line;
}

#define TAB "    "

void lexer_range_error(const char *message, int start_col, int end_col, int line) {
    fprintf(stderr, ANSI_COLOR_RED "Json Parser Error:" ANSI_COLOR_RESET);
    fprintf(stderr, " %s at line %d column %d\n", message, line, start_col);
    fprintf(stderr, "%*c%d |", 4, ' ', line);

    int *buf_pos = get_buf_line_pos(lexer.line);
    // print from start to where the highligted code is
    print_slice(stderr, lexer.buffer, buf_pos[0], buf_pos[0] + start_col);
    // print the highligted code
    print_slice_colored(
        stderr,
        lexer.buffer,
        buf_pos[0] + start_col,
        buf_pos[0] + end_col,
        ANSI_RED
    );
    // print the right side after the highligted code
    print_slice(stderr, lexer.buffer, buf_pos[0] + end_col, buf_pos[1]);
    // add a new line at the end
    putc('\n', stderr);

    // error mark
    int line_chr_len = line / 10;
    int padding = 6;
    // spaces before the | 
    fprintf(stderr, "%*c|", line_chr_len + padding, ' ');
    fprintf(stderr, "%*c" ANSI_COLOR_RED "^", start_col, ' ');

    for(int i = 0; i < end_col - start_col - 1; i++) {
        putc('~', stderr);
    }

    fprintf(stderr, ANSI_COLOR_RESET "\n");

    lexer.had_errors = true;
}

void lexer_init(char *buffer) {
    lexer.buffer = buffer;
    lexer.line = 1;
    lexer.col = 0;
}

bool lexer_is_at_end() {
    return lexer.cursor >= strlen(lexer.buffer);
}

char lexer_advance() {
    lexer.col++;
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
    Token token = {type, lexeme};
    da_append(&lexer.tokens, token);
}

void lexer_string() {
    int start_col = lexer.col - 1;
    String str = {0};
    while(!lexer_is_at_end() && lexer_peek() != '"' && lexer_peek() != '\n') {
        string_append_chr(&str, lexer_advance());
    }

    if(!lexer_match('"')) {
        lexer_range_error("Expected terminating \"", start_col, lexer.col, lexer.line);
        goto end;
    }

    lexer_add_tkn(STRING_TKN, string_dump(&str));
end:
    string_free(&str);
}

void lexer_number(char c) {
    String number = {0};
    string_append_chr(&number, c);

    if(c == '-' && !isdigit(lexer_peek())) {
        lexer_range_error("Expected digit after \"-\"", lexer.col - 1, lexer.col, lexer.line);
        goto end;
    }

    while(!lexer_is_at_end() && isdigit(lexer_peek())) {
        string_append_chr(&number, lexer_advance());
    }

    if(lexer_match('.')) {
        string_append_chr(&number, '.');

        if(!isdigit(lexer_peek())) {
            lexer_range_error("Expected digit after \".\"", lexer.col - 1, lexer.col, lexer.line);
            goto end;
        }

        while(!lexer_is_at_end() && isdigit(lexer_peek())) {
            string_append_chr(&number, lexer_advance());
        }
    }

    if(lexer_match('e')) {
        string_append_chr(&number, 'e');

        if(!isdigit(lexer_peek())) {
            lexer_range_error("Expected digit after \"e\"", lexer.col - 1, lexer.col, lexer.line);
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
    int start_col = lexer.col - 1;
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
        lexer_range_error(message, start_col, lexer.col, lexer.line);
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
                lexer.col = 0;
                break;
            case '\t':
            case ' ':
                continue;
            default: {
                if(isalpha(c)) {
                    lexer_keyword(c);
                } else if(isdigit(c) || c == '-') {
                    lexer_number(c);
                } else {
                    const char *msg = text_format("Unexpected character \"%c\"", c);
                    lexer_range_error(msg, lexer.col - 1, lexer.col, lexer.line);
                }
            }
        }
    }

    return lexer.tokens;
}

bool lexer_had_errors() {
    return lexer.had_errors;
}
