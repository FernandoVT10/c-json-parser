#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "parser_internal.h"
#include "json.h"

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
    char *buffer = read_file("./test/test.json");

    JsonValue val = json_parse_buffer(buffer);

    if(json_is_null(val)) {
        return 1;
    }

    json_print(val, 0);
    json_free(val);

    return 0;
}
