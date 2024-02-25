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
    TT_STRING,
    TT_COUNT,
} Token_Type;
    
char *token_types[TT_COUNT] = {"none", "write", "string"};
    
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
    
// Parsing
typedef enum {
    VAR_STRING,
    VAR_INT,
} Var_Type;

typedef union {
    String_View string;
    int integer;
} Var_Value;

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

typedef struct Node {
    struct Node *left;
    struct Node *right;    
    Node_Type type;
    Node_Value value;
} Node;

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
        printf("%zu:%zu: %s, "View_Print"\n", arr.data[i].loc.row, arr.data[i].loc.col, token_types[arr.data[i].type], View_Arg(arr.data[i].value));
    }
}
    
void print_tree(Node *node) {
    if(node == NULL) return;
    printf("%s: ", node_types[node->type]);
    switch(node->type) {
        case TYPE_NATIVE:
            printf("%d ", node->value.native.type);
            switch(node->value.native.args.data[0].type) {
                case VAR_STRING:
                    printf(View_Print"\n", View_Arg(node->value.native.args.data[0].value.string));            
                    break;
                case VAR_INT:
                    printf("%d\n", node->value.native.args.data[0].value.integer);
                    break;
            }
            break;
        default:
            printf("\n");
            break;
    }
    print_tree(node->left);
    print_tree(node->right);
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
            token.value = view_create(word.data, word.count);
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
    node->left = NULL;
    node->right = NULL;
    node->type = type;
    return node;
}
    
Node *parse(Token_Arr tokens) {
    Node *root = create_node(TYPE_ROOT);
    while(tokens.count > 0) {
        switch(tokens.data[0].type) {
            case TT_WRITE:
                Node *node = create_node(TYPE_NATIVE);            
                expect_token(&tokens, TT_STRING);            
                Native_Call call = {0};
                Arg arg = (Arg){.type=VAR_STRING, .value={tokens.data[0].value}};
                DA_APPEND(&call.args, arg);
                call.type = NATIVE_WRITE;
                node->value.native = call;
                root->left = node;
                break;
            case TT_STRING:
            case TT_NONE:
            case TT_COUNT:
                ASSERT(false, "unreachable");
        }
        token_consume(&tokens);
    }
    root->right = create_node(TYPE_NATIVE);
    Native_Call call = {0};
    call.type = NATIVE_EXIT;
    Arg arg = (Arg){.type=VAR_INT};
    arg.value.integer = 0;
    DA_APPEND(&call.args, arg);
    root->right->value.native = call;
    return root;
}
    
void generate(Node *node) {
    if(node == NULL) return;
    switch(node->type) {
        case TYPE_NATIVE:
            switch(node->value.native.type) {
                case NATIVE_WRITE:
                    ASSERT(node->value.native.args.count == 1, "too many arguments");
                    if(node->value.native.args.data[0].type != VAR_STRING) exit(1);
                    printf("push_str \""View_Print"\"\n", View_Arg(node->value.native.args.data[0].value.string));
                    printf("push %d\n", STDOUT);
                    printf("native %d\n", node->value.native.type);
                    break;
                case NATIVE_EXIT:
                    ASSERT(node->value.native.args.count == 1, "too many arguments");
                    if(node->value.native.args.data[0].type != VAR_INT) exit(1);
                    printf("push %d\n", node->value.native.args.data[0].value.integer);
                    printf("native %d\n", node->value.native.type);
                    break;           
                default:
                    ASSERT(false, "unreachable");
            }
            break;
        default:
            break;
    }    
    generate(node->left);
    generate(node->right);
}

int main() {
    String_View view = read_file_to_view("cano.cano");
    Token_Arr tokens = lex(view);
    Node *root = parse(tokens);
    generate(root);
}
