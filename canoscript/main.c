#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define VIEW_IMPLEMENTATION
#include "view.h"

#define DATA_START_CAPACITY 16

#define ASSERT(cond, ...) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "%s:%d: ASSERTION FAILED: ", __FILE__, __LINE__); \
            fprintf(stderr, __VA_ARGS__); \
            fprintf(stderr, "\n"); \
            exit(1); \
        } \
    } while (0)

#define DA_APPEND(da, item) do {                                                       \
    if ((da)->count >= (da)->capacity) {                                               \
        (da)->capacity = (da)->capacity == 0 ? DATA_START_CAPACITY : (da)->capacity*2; \
        (da)->data = realloc((da)->data, (da)->capacity*sizeof(*(da)->data));       \
        ASSERT((da)->data != NULL, "outta ram");                               \
    }                                                                                  \
    (da)->data[(da)->count++] = (item);                                               \
} while (0)
    
typedef struct {
    char *data;
    size_t count;
    size_t capacity;
} Dynamic_Str;

typedef enum {
    TT_NONE = 0,
    TT_WRITE,
    TT_STRING,
    TT_COUNT,
} Token_Type;
    
char *types[TT_COUNT] = {"none", "write", "string"};
    
typedef struct {
    size_t row;
    size_t col;
    char *filename;
} Location;

typedef struct {
    Token_Type type;
    Location loc;
    String_View value;
} Token;
    
typedef struct {
    Token *data;
    size_t count;
    size_t capacity;
} Token_Arr;

String_View read_file_to_view(char *filename) {
    FILE *file = fopen(filename, "r");
    
    fseek(file, 0, SEEK_END);
    size_t len = ftell(file);
    fseek(file, 0, SEEK_SET);    
    
    char *data = malloc(sizeof(char)*len);
    fread(data, sizeof(char), len, file);
    
    fclose(file);
    return (String_View){.data=data, .len=len};
}
    
bool isword(char c) {
    return isalpha(c) || isdigit(c) || c == '_';
}
    
Token_Type get_token_type(char *str, size_t str_s) {
    if(strncmp(str, "write", str_s) == 0) {
        return TT_WRITE;
    }
    return TT_NONE;
}
    
void print_token_arr(Token_Arr arr) {
    for(size_t i = 0; i < arr.count; i++) {
        printf("%zu: %s, "View_Print"\n", i, types[arr.data[i].type], View_Arg(arr.data[i].value));
    }
}

Token_Arr lex(String_View view) {
    Token_Arr tokens = {0};
    while(view.len > 0) {
        if(isalpha(*view.data)) {
            Dynamic_Str word = {0};    
            DA_APPEND(&word, *view.data);                    
            view = view_chop_left(view);
            while(view.len > 0 && isword(*view.data)) {
                DA_APPEND(&word, *view.data);            
                view = view_chop_left(view);    
            }
            Token token = {0};
            token.type = get_token_type(word.data, word.count);
            if(token.type == TT_NONE) exit(1);
            DA_APPEND(&tokens, token);     
            free(word.data);
        } else if(*view.data == '"') {
            Dynamic_Str word = {0};
            view = view_chop_left(view);
            while(view.len > 0 && *view.data != '"') {
                DA_APPEND(&word, *view.data);
                view = view_chop_left(view);
            }
            if(view.len == 0 && *view.data != '"') exit(1);
            Token token = {0};
            token.type = TT_STRING;
            token.value = view_create(word.data, word.count);
            DA_APPEND(&tokens, token);
        }
        view = view_chop_left(view);
    }
    return tokens;
}

int main() {
    String_View view = read_file_to_view("cano.cano");
    Token_Arr tokens = lex(view);
    print_token_arr(tokens);
}
