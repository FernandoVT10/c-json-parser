#ifndef JSON_H
#define JSON_H

#include <stdio.h>
#include <stdbool.h>

typedef struct JsonArray JsonArray;
typedef struct JsonObject JsonObject;
typedef struct JsonObjectItem JsonObjectItem;

typedef enum {
    JSON_STRING,
    JSON_NUMBER,
    JSON_BOOL,
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_NULL,
} JsonValueType;

typedef struct {
    JsonValueType type;
    union {
        char *str;
        JsonArray *arr;
        JsonObject *obj;
        double number;
        bool boolean;
    };
} JsonValue;

struct JsonObjectItem {
    char *key;
    JsonValueType type;
    JsonValue value;
    JsonObjectItem *next;
};

struct JsonObject {
    JsonObjectItem *head;
    JsonObjectItem *tail;
    size_t count;
};

struct JsonArray {
    JsonValue *items;
    size_t capacity;
    size_t count;
};

// Parsing functions
JsonValue json_parse_buffer(char *buffer);

// Value creation functions
JsonValue json_object(JsonObject *obj);
JsonValue json_null();

// Value checkers
bool json_is_null(JsonValue value);

// Object manipulation functions
JsonObject* json_object_new();
void json_object_set(JsonObject *obj, char *key, JsonValue value);

// Array manipulation functions
JsonArray *json_array_new();
void json_array_push(JsonArray *arr, JsonValue value);

void json_free(JsonValue val);

// prints the json value with an indentation
void json_print(JsonValue val, int indent);

#endif // JSON_H
