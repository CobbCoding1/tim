#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define VIEW_IMPLEMENTATION
#include "view.h"

#define DATA_START_CAPACITY 16

#define NATIVE_OPEN 0
#define NATIVE_WRITE 1
#define NATIVE_EXIT 60

#define STDOUT 1

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


// Lexing
typedef enum {
    TT_NONE = 0,
    TT_WRITE,
    TT_EXIT,
    TT_STRING,
    TT_INT,
    TT_COUNT,
} Token_Type;
    
char *token_types[TT_COUNT] = {"none", "write", "exit", "string", "integer"};
    
typedef struct {
    size_t row;
    size_t col;
    char *filename;
} Location;
    
typedef union {
    String_View string;
    int integer;
} Var_Value;

typedef struct {
    Token_Type type;
    Location loc;
    Var_Value value;
} Token;
    
typedef struct {
    Token *data;
    size_t count;
    size_t capacity;
} Token_Arr;
    
// Parsing
typedef enum {
    VAR_STRING,
    VAR_INT,
} Var_Type;

typedef struct {
    Var_Type type;
    Var_Value value; 
} Arg;
    
typedef struct {
    Arg *data;
    size_t count;
    size_t capacity;
} Args;

typedef struct {
    Args args;
    int type;
} Native_Call;

typedef enum {
    TYPE_ROOT = 0,
    TYPE_NATIVE,
    TYPE_COUNT,
} Node_Type;
    
char *node_types[TYPE_COUNT] = {"root", "native"};

typedef union {
    Native_Call native;
} Node_Value;

typedef struct {
    Node_Type type;
    Node_Value value;
} Node;
    
typedef struct {
    Node *data;
    size_t count;
    size_t capacity;
} Nodes;
    
void usage(char *file) {
    fprintf(stderr, "usage: %s <filename.cano>\n", file);
    exit(1);
}

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
    } else if(strncmp(str, "exit", str_s) == 0) {
        return TT_EXIT;
    }
    return TT_NONE;
}
    
void print_token_arr(Token_Arr arr) {
    for(size_t i = 0; i < arr.count; i++) {
        printf("%zu:%zu: %s, "View_Print"\n", arr.data[i].loc.row, arr.data[i].loc.col, token_types[arr.data[i].type], View_Arg(arr.data[i].value.string));
    }
}

Token_Arr lex(String_View view) {
    size_t row = 1;
    Token_Arr tokens = {0};
    char *start = view.data;
    while(view.len > 0) {
        if(isalpha(*view.data)) {
            Token token = {0};
            token.loc = (Location){
                .row = row,
                .col = view.data-start+1,
            };
            Dynamic_Str word = {0};    
            DA_APPEND(&word, *view.data);                    
            view = view_chop_left(view);
            while(view.len > 0 && isword(*view.data)) {
                DA_APPEND(&word, *view.data);            
                view = view_chop_left(view);    
            }
            token.type = get_token_type(word.data, word.count);
            if(token.type == TT_NONE) exit(1);
            DA_APPEND(&tokens, token);     
            free(word.data);
        } else if(*view.data == '"') {
            Token token = {0};
            token.loc = (Location){
                .row = row,
                .col = view.data-start+1,
            };
            Dynamic_Str word = {0};
            view = view_chop_left(view);
            while(view.len > 0 && *view.data != '"') {
                DA_APPEND(&word, *view.data);
                view = view_chop_left(view);
            }
            if(view.len == 0 && *view.data != '"') exit(1);
            token.type = TT_STRING;
            token.value.string = view_create(word.data, word.count);
            DA_APPEND(&tokens, token);
        } else if(isdigit(*view.data)) {
            Token token = {0};
            token.loc = (Location){
                .row = row,
                .col = view.data-start+1,
            };
        
            Dynamic_Str num = {0};
            while(view.len > 0 && isdigit(*view.data)) {
                DA_APPEND(&num, *view.data);
                view = view_chop_left(view);
            }
            DA_APPEND(&num, '\0');
            token.type = TT_INT;
            token.value.integer = atoi(num.data);
            DA_APPEND(&tokens, token);
        } else if(*view.data == '\n') {
            row++;
        }
        view = view_chop_left(view);
    }
    return tokens;
}
    
void token_consume(Token_Arr *tokens) {
    if(tokens->count == 0) exit(1);
    tokens->count--;
    tokens->data++;
}

void expect_token(Token_Arr *tokens, Token_Type type) {
    token_consume(tokens);
    if(tokens->data[0].type != type) exit(1);
}

Node *create_node(Node_Type type) {
    Node *node = malloc(sizeof(Node));
    node->type = type;
    return node;
}
    
Nodes parse(Token_Arr tokens) {
    Nodes root = {0};
    while(tokens.count > 0) {
        switch(tokens.data[0].type) {
            case TT_WRITE: {
                Node node = {.type = TYPE_NATIVE};            
                expect_token(&tokens, TT_STRING);            
                Native_Call call = {0};
                Arg arg = (Arg){.type=VAR_STRING, .value.string=tokens.data[0].value.string};
                DA_APPEND(&call.args, arg);
                call.type = NATIVE_WRITE;
                node.value.native = call;
                DA_APPEND(&root, node);
            } break;
            case TT_EXIT: {
                Node node = {.type = TYPE_NATIVE};            
                expect_token(&tokens, TT_INT);            
                Native_Call call = {0};
                Arg arg = (Arg){.type=VAR_INT, .value.integer=tokens.data[0].value.integer};
                DA_APPEND(&call.args, arg);
                call.type = NATIVE_EXIT;
                node.value.native = call;
                DA_APPEND(&root, node);
            } break;
            case TT_STRING:
            case TT_INT:            
            case TT_NONE:
            case TT_COUNT:
                ASSERT(false, "unreachable");
        }
        token_consume(&tokens);
    }
    return root;
}

void strip_off_dot(char *str) {
    while(*str != '\0' && *str != '.') {
        str++;
    }
    *str = '\0';
}

char *append_tasm_ext(char *filename) {
    size_t filename_s = strlen(filename);
    char *output = malloc(sizeof(char)*filename_s);
    strncpy(output, filename, filename_s);
    strip_off_dot(output);
    char *output_filename = malloc(sizeof(char)*strlen(output)+sizeof(".tasm"));
    sprintf(output_filename, "%s.tasm", output);
    return output_filename;
}
    
void generate(Nodes nodes, char *filename) {
    char *output = append_tasm_ext(filename);
    FILE *file = fopen(output, "w");
    for(size_t i = 0; i < nodes.count; i++) {
        Node *node = &nodes.data[i];
        switch(node->type) {
            case TYPE_NATIVE:
                switch(node->value.native.type) {
                    case NATIVE_WRITE:
                        ASSERT(node->value.native.args.count == 1, "too many arguments");
                        if(node->value.native.args.data[0].type != VAR_STRING) exit(1);
                        fprintf(file, "push_str \""View_Print"\"\n", View_Arg(node->value.native.args.data[0].value.string));
                        fprintf(file, "push %d\n", STDOUT);
                        fprintf(file, "native %d\n", node->value.native.type);
                        break;
                    case NATIVE_EXIT:
                        ASSERT(node->value.native.args.count == 1, "too many arguments");
                        if(node->value.native.args.data[0].type != VAR_INT) exit(1);
                        fprintf(file, "push %d\n", node->value.native.args.data[0].value.integer);
                        fprintf(file, "native %d\n", node->value.native.type);
                        break;           
                    default:
                        ASSERT(false, "unreachable");
                }
                break;
            default:
                break;
        }    
    }
    fprintf(file, "push 0\n");
    fprintf(file, "native 60\n");    
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        usage(argv[0]);
    }
    char *filename = argv[1];
    String_View view = read_file_to_view(filename);
    Token_Arr tokens = lex(view);
    Nodes root = parse(tokens);
    generate(root, filename);
}
