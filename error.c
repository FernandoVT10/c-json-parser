#include <stdio.h>
#include <string.h>

#include "error.h"

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"

typedef enum {
    ANSI_RED,
} AnsiColor;

void print_slice(FILE *stream, const char *text, int start, int end) {
    if(start < 0) start = 0;

    size_t len = strlen(text);
    if(end > len) end = len - 1;
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

int *get_buf_line_pos(const char *buffer, int line) {
    static int buf_line[2];
    buf_line[0] = 0;
    buf_line[1] = 0;

    int x = 0;
    while(line > 1 && x < strlen(buffer)) {
        if(buffer[x] == '\n') line--;
        buf_line[0]++;
        x++;
    }

    buf_line[1] = buf_line[0];
    while(buf_line[1] < strlen(buffer) && buffer[buf_line[1]] != '\n') {
        buf_line[1]++;
    }

    return buf_line;
}

#define TAB "    "

void syntax_error(const char *buffer, const char *message, int start_col, int end_col, int line) {
    fprintf(stderr, ANSI_COLOR_RED "Json Parser Error:" ANSI_COLOR_RESET);
    fprintf(stderr, " %s at line %d column %d\n", message, line, start_col);
    fprintf(stderr, "%*c%d |", 4, ' ', line);

    int *buf_pos = get_buf_line_pos(buffer, line);
    // print from start to where the highligted code is
    print_slice(stderr, buffer, buf_pos[0], buf_pos[0] + start_col);
    // print the highligted code
    print_slice_colored(
        stderr,
        buffer,
        buf_pos[0] + start_col,
        buf_pos[0] + end_col,
        ANSI_RED
    );
    // print the right side after the highligted code
    print_slice(stderr, buffer, buf_pos[0] + end_col, buf_pos[1]);
    // add a new line at the end
    putc('\n', stderr);

    // error mark
    int line_chr_len = line / 10;
    int padding = 6;
    // spaces before the |
    fprintf(stderr, "%*c|", line_chr_len + padding, ' ');
    fprintf(stderr, "%*c" ANSI_COLOR_RED "^", 120, ' ');

    for(int i = 0; i < end_col - start_col - 1; i++) {
        putc('~', stderr);
    }

    fprintf(stderr, ANSI_COLOR_RESET "\n");
}
