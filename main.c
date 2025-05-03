#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "lexer.h"
#include "parser.h"

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
    char *buffer = read_file("./test.json");
    lexer_init(buffer);
    Tokens tokens = lexer_scan();
    if(lexer_had_errors()) {
        return 1;
    }
    JsonObject *obj = json_parse(tokens);
    json_object_print(obj, 0);

    return 0;
}
