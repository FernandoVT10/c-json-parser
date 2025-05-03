#include <stdbool.h>

#include "json_types.h"
#include "stdlib.h"
#include "string.h"
#include "cTooling.h"

void json_print_value(const char *key, JsonValue value, int indent);

void print_indentation(int indent) {
    for(int i = 0; i < indent; i++) {
        putchar(' ');
    }
}

JsonObjectItem* json_object_get(JsonObject *obj, char *key) {
    JsonObjectItem *item = obj->head;
    while(item != NULL) {
        if(strcmp(key, item->key) == 0) {
            return item;
        }

        item = item->next;
    }

    return NULL;
}

void json_object_set(JsonObject *obj, char *key, JsonValue value) {
    JsonObjectItem *item = json_object_get(obj, key);

    if(item != NULL) {
        item->value = value;
    } else {
        item = malloc(sizeof(JsonObjectItem));
        item->key = strdup(key);
        item->value = value;
        item->next = NULL;

        if(obj->count > 0) {
            obj->tail->next = item;
            obj->tail = item;
        } else {
            obj->head = item;
            obj->tail = item;
        }

        obj->count++;
    }
}

bool json_object_has(JsonObject *obj, char *key) {
    return json_object_get(obj, key) != NULL;
}

JsonObject* json_object_new() {
    JsonObject *obj = malloc(sizeof(JsonObject));
    bzero(obj, sizeof(JsonObject));
    return obj;
}

void json_inner_object_print(JsonObject *obj, int indent) {
    JsonObjectItem *item = obj->head;
    printf("{\n");
    while(item != NULL) {
        json_print_value(item->key, item->value, indent + 4);
        item = item->next;
    }
    print_indentation(indent);
    printf("}\n");
}

void json_print_value(const char *key, JsonValue value, int indent) {
    print_indentation(indent);
    switch(value.type) {
        case JSON_STRING:
            printf("%s => \"%s\"\n", key, (char*)value.ptr);
            break;
        case JSON_NUMBER:
            printf("%s => %lf\n", key, value.literal);
            break;
        case JSON_BOOL:
            char *val = value.literal == 1 ? "true" : "false";
            printf("%s => %s\n", key, val);
            break;
        case JSON_OBJECT:
            printf("%s => ", key);
            json_inner_object_print((JsonObject*)value.ptr, indent);
            break;
        case JSON_ARRAY:
            printf("%s => ", key);
            json_array_print((JsonArray*)value.ptr, indent);
            break;
        case JSON_NULL:
            printf("%s => %s\n", key, "null");
            break;
    }
}


void json_object_print(JsonObject *obj, int indent) {
    JsonObjectItem *item = obj->head;
    print_indentation(indent);
    printf("{\n");
    while(item != NULL) {
        json_print_value(item->key, item->value, indent + 4);
        item = item->next;
    }
    print_indentation(indent);
    printf("}\n");
}

JsonArray *json_array_new() {
    JsonArray *arr = malloc(sizeof(JsonArray));
    bzero(arr, sizeof(JsonArray));
    return arr;
}

void json_array_push(JsonArray *arr, JsonValue value) {
    JsonArrayItem *item = malloc(sizeof(JsonArrayItem));
    item->value = value;
    item->next = NULL;

    if(arr->count > 0) {
        arr->tail->next = item;
    } else {
        arr->head = item;
    }

    arr->tail = item;
    arr->count++;
}

void json_array_print(JsonArray *arr, int indent) {
    JsonArrayItem *item = arr->head;
    printf("[\n");
    int i = 0;
    while(item != NULL) {
        json_print_value(text_format("[%d]", i), item->value, indent + 4);
        i++;
        item = item->next;
    }
    print_indentation(indent);
    printf("]\n");
}
