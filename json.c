#include <stdlib.h>

#include "parser_internal.h"
#include "json.h"
#include "cTooling.h"

JsonValue json_parse_buffer(char *buffer) {
    Lexer lexer = {
        .buffer = buffer,
    };
    lexer_init(&lexer);
    lexer_scan(&lexer);

    if(lexer.had_errors) {
        lexer_free(&lexer);
        return json_null();
    }

    Parser parser = {
        .buffer = buffer,
        .tokens = lexer.tokens,
    };
    JsonValue value = parser_parse_tokens(&parser);
    lexer_free(&lexer);

    if(parser.had_errors) {
        json_free(value);
        return json_null();
    }

    return value;
}

JsonValue json_object(JsonObject *obj) {
    return (JsonValue){
        .type = JSON_OBJECT,
        .obj = obj,
    };
}

JsonValue json_null() {
    return (JsonValue){
        .type = JSON_NULL,
    };
}

bool json_is_null(JsonValue value) {
    return value.type == JSON_NULL;
}

JsonObject* json_object_new() {
    JsonObject *obj = malloc(sizeof(JsonObject));
    bzero(obj, sizeof(JsonObject));
    return obj;
}

static JsonObjectItem* object_find_item(JsonObject *obj, char *key) {
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
    JsonObjectItem *item = object_find_item(obj, key);

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

JsonArray *json_array_new() {
    JsonArray *arr = malloc(sizeof(JsonArray));
    bzero(arr, sizeof(JsonArray));
    return arr;
}

void json_array_push(JsonArray *arr, JsonValue value) {
    da_append(arr, value);
}

void json_free(JsonValue val) {
    switch(val.type) {
        case JSON_STRING: free(val.str); break;
        case JSON_OBJECT:
            JsonObjectItem *item = val.obj->head;
            while(item != NULL) {
                json_free(item->value);
                free(item->key);

                JsonObjectItem *old_item = item;
                item = item->next;
                free(old_item);
            }
            free(val.obj);
            break;
        case JSON_ARRAY:
            for(size_t i = 0; i < val.arr->count; i++) {
                json_free(val.arr->items[i]);
            }
            da_free(val.arr);
            free(val.arr);
            break;
        case JSON_NUMBER:
        case JSON_BOOL:
        case JSON_NULL:
            break;
    }
}

static void print_indentation(int indent) {
    for(int i = 0; i < indent; i++) {
        putchar(' ');
    }
}

static void print_object(JsonObject *obj, int indent) {
    JsonObjectItem *item = obj->head;

    printf("{\n");
    while(item != NULL) {
        int local_indent = indent + 4;
        print_indentation(local_indent);
        printf("[%s] => ", item->key);
        json_print(item->value, local_indent);
        item = item->next;
    }
    print_indentation(indent);
    printf("}\n");
}

static void print_array(JsonArray *arr, int indent) {
    printf("[\n");
    for(size_t i = 0; i < arr->count; i++) {
        int local_indent = indent + 4;
        print_indentation(local_indent);
        printf("[%lu] => ", i);
        json_print(arr->items[i], local_indent);
    }
    print_indentation(indent);
    printf("]\n");
}

void json_print(JsonValue val, int indent) {
    switch(val.type) {
        case JSON_STRING:
            printf("\"%s\"\n", (char*)val.str);
            break;
        case JSON_NUMBER:
            printf("%lf\n", val.number);
            break;
        case JSON_BOOL:
            char *text = val.boolean ? "true" : "false";
            printf("%s\n", text);
            break;
        case JSON_OBJECT:
            print_object(val.obj, indent);
            break;
        case JSON_ARRAY:
            print_array(val.arr, indent);
            break;
        case JSON_NULL:
            printf("null\n");
            break;
    }
}
