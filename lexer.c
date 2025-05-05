#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "cTooling.h"
#include "lexer.h"

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"

Lexer lexer = {0};
    // public syntaxError(message: string, line: number, col: number | number[]): void {
    //     const bufLine = this.getBufLine(line);
    //
    //     let error = "";
    //
    //     const spaces = (n: number): string => "".padStart(n, " ");
    //
    //     const colNumber = Array.isArray(col) ? col[0] : col;
    //     // filepath:line
    //     error += formatString(ANSIColor.RED, `${this.filePath}:${line}:${colNumber}`) + "\n";
    //
    //     // ERROR: message
    //     error += `${formatString(ANSIColor.RED, "ERROR:")} ${message}\n`;
    //
    //     let errorStart: number, errorEnd: number;
    //
    //     if(Array.isArray(col)) {
    //         errorStart = col[0] - 1;
    //         errorEnd = col[1] - 1;
    //     } else {
    //         errorStart = col - 1;
    //         errorEnd = col;
    //     }
    //
    //     // the line of code where the error is
    //     const left = bufLine.slice(0, errorStart);
    //     const highlightedCode = formatString(ANSIColor.RED, bufLine.slice(errorStart, errorEnd));
    //     const right = bufLine.slice(errorEnd);
    //     error += `${spaces(4)}${line} |${left}${highlightedCode}${right}\n`;
    //
    //     // A mark pointing the exact place where the error was found
    //     const lineLen = line.toString().length;
    //     const errorMark = "^".padEnd(errorEnd - errorStart, "~");
    //     error += `${spaces(4 + lineLen)} |${spaces(errorStart)}${formatString(ANSIColor.RED, errorMark)}`;
    //
    //     this.logger.error(error);
    //     this.hadErrors = true;
    // }

typedef enum {
    ANSI_RED,
} AnsiColor;

void print_slice(const char *text, int start, int end) {
    for(size_t i = start; i < end; i++) {
        putchar(text[i]);
    }
}

void print_slice_colored(const char *text, int start, int end, AnsiColor color) {
    switch(color) {
        case ANSI_RED:
            printf(ANSI_COLOR_RED);
            print_slice(text, start, end);
            printf(ANSI_COLOR_RESET);
        break;
    }
}

int *get_buf_line_pos() {
    static int buf_line[2];
    buf_line[0] = 0;
    buf_line[1] = 0;

    int x = 0;
    int line = lexer.line;
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

void lexer_error(const char *message) {
    // TODO: do a better error logging
    fprintf(stderr, ANSI_COLOR_RED "Json Parser Error:" ANSI_COLOR_RESET);
    fprintf(stderr, " %s at line %d column %d\n", message, lexer.line, lexer.col);

    int *line_pos = get_buf_line_pos();
    printf("    %d |", lexer.line);
    print_slice(lexer.buffer, line_pos[0], line_pos[0] + 10);
    print_slice_colored(lexer.buffer, line_pos[0] + 10, line_pos[0] + 16, ANSI_RED);
    print_slice(lexer.buffer, line_pos[0] + 16, line_pos[1]);
    putchar('\n');
    printf("      |          " ANSI_COLOR_RED "^~~~~~" ANSI_COLOR_RESET "\n");

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
    String str = {0};
    while(!lexer_is_at_end() && lexer_peek() != '"' && lexer_peek() != '\n') {
        string_append_chr(&str, lexer_advance());
    }

    if(!lexer_match('"')) {
        lexer_error("Expected terminating \"");
        goto end;
    }

    lexer_add_tkn(STRING_TKN, string_dump(&str));
end:
    string_free(&str);
}

void lexer_number(char c) {
    String number = {0};
    string_append_chr(&number, c);

    while(!lexer_is_at_end() && isdigit(lexer_peek())) {
        string_append_chr(&number, lexer_advance());
    }

    if(lexer_match('.')) {
        string_append_chr(&number, '.');

        if(!isdigit(lexer_peek())) {
            lexer_error("Expected digit");
            goto end;
        }

        while(!lexer_is_at_end() && isdigit(lexer_peek())) {
            string_append_chr(&number, lexer_advance());
        }
    }

    if(lexer_match('e')) {
        string_append_chr(&number, 'e');

        if(!isdigit(lexer_peek())) {
            lexer_error("Expected digit");
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
        lexer_error(text_format("Unknown keyword \"%s\"", keyword));
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
                } else if(isdigit(c)) {
                    lexer_number(c);
                } else {
                    lexer_error(text_format("Unexpected character \"%c\"", c));
                }
            }
        }
    }

    return lexer.tokens;
}

bool lexer_had_errors() {
    return lexer.had_errors;
}
