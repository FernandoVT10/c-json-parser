#ifndef JSON_TYPES_H
#define JSON_TYPES_H

#include "stdio.h"

typedef enum {
    JSON_STRING,
    JSON_NUMBER,
    JSON_BOOL,
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_NULL,
} JsonValueType;

typedef struct JsonObjectItem JsonObjectItem;

typedef struct {
    JsonValueType type;
    union {
        void *ptr; // pointer value for strings, objects, and arrays
        double literal; // literal value for numbers and bools
    };
} JsonValue;

struct JsonObjectItem {
    char *key;
    JsonValueType type;
    JsonValue value;
    JsonObjectItem *next;
};

typedef struct {
    JsonObjectItem *head;
    JsonObjectItem *tail;
    size_t count;
} JsonObject;

typedef struct JsonArrayItem JsonArrayItem;

struct JsonArrayItem {
    JsonValue value;
    JsonArrayItem *next;
};

typedef struct {
    JsonArrayItem *head;
    JsonArrayItem *tail;
    size_t count;
} JsonArray;

void json_object_set(JsonObject *obj, char *key, JsonValue value);
JsonObjectItem *json_object_get(JsonObject *obj, char *key);
bool json_object_has(JsonObject *obj, char *key);
JsonObject* json_object_new();
void json_object_destroy(JsonObject *obj);
void json_object_print(JsonObject *obj, int indent);

JsonArray *json_array_new();
void json_array_push(JsonArray *arr, JsonValue value);
void json_array_print(JsonArray *arr, int indent);

#endif // JSON_TYPES_H
