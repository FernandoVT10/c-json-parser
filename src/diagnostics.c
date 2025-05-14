#include "parser_internal.h"
#include "string.h"

#define MIN(a, b) (a > b ? b : a)

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define SLICE_SIZE 1024

static void print_slice(const char *text, int start, int end) {
    if(start < 0) start = 0;

    size_t len = strlen(text);
    if(end > len) end = len - 1;

    for(size_t i = start; i < end; i++) {
        putc(text[i], stderr);
    }
}

static const void print_code_line(ErrorSrc err) {
    int cur_line = 1;
    int start_pos = 0;
    while(start_pos < strlen(err.src) && cur_line < err.line) {
        if(err.src[start_pos] == '\n') cur_line++;
        start_pos++;
    }

    int end_pos = start_pos;
    while(end_pos < strlen(err.src)) {
        if(err.src[end_pos] == '\n') break;
        end_pos++;
    }

    // LEFT
    print_slice(err.src, start_pos, start_pos + err.col.start);

    // HIGHLIGHT
    fprintf(stderr, ANSI_COLOR_RED);
    print_slice(err.src, start_pos + err.col.start, start_pos + err.col.end);
    fprintf(stderr, ANSI_COLOR_RESET);

    // RIGHT
    print_slice(err.src, start_pos + err.col.end, end_pos);
}

static void repeat_char(int n, char c) {
    while(n > 0) {
        putc(c, stderr);
        n--;
    }
}

void print_range_error(const char *msg, ErrorSrc err) {
    if(err.eof) {
        fprintf(
            stderr,
            ANSI_COLOR_RED "Json Parser Error: " ANSI_COLOR_RESET "%s at the end of file\n",
            msg
        );
    } else {
        fprintf(
            stderr,
            ANSI_COLOR_RED "Json Parser Error: " ANSI_COLOR_RESET "%s at line %d column %d\n",
            msg,
            err.line,
            // columns in editors start at 1 instead of 0
            err.col.start + 1
        );
    }

    int indentation = 4;

    fprintf(stderr, "%*c%d |", indentation, ' ', err.line);
    print_code_line(err);
    putc('\n', stderr);

    char buf[64];
    sprintf(buf, "%d", err.line);
    int line_chr_len = strlen(buf);
    // the one comes from the space printed after the line number
    int padding = line_chr_len + indentation + 1;
    fprintf(stderr, "%*c|", padding, ' ');
    fprintf(stderr, ANSI_COLOR_RED "%*c", err.col.start + 1, '^');
    int error_length = err.col.end - err.col.start - 1;
    repeat_char(error_length, '~');
    fprintf(stderr, ANSI_COLOR_RESET"\n");
}

void print_message_error(const char *msg) {
    fprintf(stderr, ANSI_COLOR_RED "Json Parser Error: " ANSI_COLOR_RESET "%s\n", msg);
}
