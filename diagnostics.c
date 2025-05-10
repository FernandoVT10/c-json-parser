#include "parser_internal.h"
#include "string.h"

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define SLICE_SIZE 1024

const char *get_slice(const char *text, int start, int end) {
    static char slice[SLICE_SIZE];

    if(start < 0) start = 0;

    size_t len = strlen(text);
    if(end > len) end = len - 1;

    int slice_len = end - start;
    slice_len = slice_len >= SLICE_SIZE ? SLICE_SIZE - 1 : slice_len;
    memcpy(slice, text + start, slice_len);

    slice[slice_len] = '\0';

    return slice;
}

void print_slice(const char *text, int start, int end) {
    if(start < 0) start = 0;

    size_t len = strlen(text);
    if(end > len) end = len - 1;

    for(size_t i = start; i < end; i++) {
        putc(text[i], stderr);
    }
}

void print_range_error(const char *msg, ErrorRange err) {
    fprintf(
        stderr,
        ANSI_COLOR_RED "Json Parser Error: " ANSI_COLOR_RESET "%s at line %d column %d\n",
        msg,
        err.line,
        err.col.start
    );

    int indentation = 4;

    const char *code_line = get_slice(err.src, err.col.start, err.col.end);
    fprintf(stderr, "%*c%d |%s\n", indentation, ' ', err.line, code_line);

    // int line_chr_len = line / 10;
    // fprintf(stderr, "%*c|", line_chr_len + padding, ' ');
    // fprintf(stderr, "%*c" ANSI_COLOR_RED "^", start_col, ' ');
}
