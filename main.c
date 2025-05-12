#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "parser_internal.h"

FILE *open_file(const char *path) {
    FILE *file = fopen(path, "r");
    if(file == NULL) {
        fprintf(stderr, "[ERROR] Couldn't open file %s: %s\n", path, strerror(errno));
        return NULL;
    }
    return file;
}

char* read_file(char *path) {
    FILE *file = fopen(path, "r");
    if(file == NULL) {
        fprintf(stderr, "[ERROR] Couldn't open file %s not found\n", path);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char *buffer = malloc(size * sizeof(char) + 1);
    fread(buffer, size, sizeof(char), file);
    fclose(file);

    buffer[size] = '\0';
    return buffer;
}

int main() {
    char *buffer = read_file("./test/5mb.json");
    Lexer lexer = {
        .buffer = buffer,
    };
    lexer_init(&lexer);
    lexer_scan(&lexer);

    if(lexer.had_errors) {
        lexer_destroy(&lexer);
        return 1;
    }

    Parser parser = {
        .buffer = buffer,
        .tokens = lexer.tokens,
    };
    parser_parse_tokens(&parser);

    if(parser.had_errors) {
        lexer_destroy(&lexer);
        return 1;
    }

    // json_object_print(obj, 0);

    lexer_destroy(&lexer);

    return 0;
}
