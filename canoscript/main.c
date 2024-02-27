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
        (da)->data = custom_realloc((da)->data, (da)->capacity*sizeof(*(da)->data));       \
        ASSERT((da)->data != NULL, "outta ram");                               \
    }                                                                                  \
    (da)->data[(da)->count++] = (item);                                               \
} while (0)
    
#define PRINT_ERROR(loc, str, ...)                                                 \
    do {                                                                           \
        fprintf(stderr, "%zu:%zu: error:"str"\n", loc.row, loc.col, __VA_ARGS__);  \
        exit(1);                                                                   \
    } while(0)
    
void *custom_realloc(void *ptr, size_t size) {
    void *new_ptr = realloc(ptr, size);
    ASSERT(new_ptr != NULL, "buy more ram");
    return new_ptr;
}

    
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
    TT_PLUS,
    TT_MINUS,
    TT_MULT,
    TT_DIV,
    TT_STRING,
    TT_INT,
    TT_COUNT,
} Token_Type;
    
char *token_types[TT_COUNT] = {"none", "write", "exit", "+", "-", "*", "/", "string", "integer"};
    
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
    OP_PLUS,
    OP_MINUS,
    OP_MULT,
    OP_DIV,
} Operator_Type;

typedef enum {
    PREC_1,
    PREC_2,
    PREC_COUNT,
} Precedence;

typedef struct {
    Operator_Type type;
    Precedence precedence;
} Operator;

struct Expr;    

typedef struct {
    struct Expr *lhs;
    struct Expr *rhs;
    Operator op;
} Bin_Expr;

typedef enum {
    EXPR_BIN,    
    EXPR_INT,
} Expr_Type;
    
typedef union {
    Bin_Expr bin;
    int integer;
} Expr_Value;

typedef struct Expr {
    Expr_Value value;
    Expr_Type type;   
} Expr;

typedef enum {
    VAR_STRING,
    VAR_INT,
} Var_Type;

typedef struct {
    String_View string;
    Expr *expr;
} Arg_Value;
    
typedef enum {
    ARG_STRING,
    ARG_EXPR,
} Arg_Type;

typedef struct {
    Arg_Type type;
    Arg_Value value; 
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
    TYPE_EXPR,
    TYPE_COUNT,
} Node_Type;
    
char *node_types[TYPE_COUNT] = {"root", "native"};

typedef union {
    Native_Call native;
    Expr expr;
} Node_Value;

typedef struct {
    Node_Type type;
    Node_Value value;
    Location loc;
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
    

bool is_valid_escape(char c){
    switch(c){
        case 'n':
        case 't':
        case 'v':
        case 'b':
        case 'r':
        case 'f':
        case 'a':
        case '\\':
        case '?':
        case '\'':
        case '\"':
        case '0':
            return true;
        default:
            return false;
    }
}

String_View read_file_to_view(char *filename) {
    FILE *file = fopen(filename, "r");
    
    fseek(file, 0, SEEK_END);
    size_t len = ftell(file);
    fseek(file, 0, SEEK_SET);    
    
    char *data = custom_realloc(NULL, sizeof(char)*len);
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
    
bool is_operator(char c) {
    switch(c) {
        case '+':
        case '-':
        case '*':
        case '/':
            return true;        
        default:
            return false;
    }
}
    
Token create_operator_token(size_t row, size_t col, char c) {
    Token token = {0};
    token.loc = (Location){
        .row = row,
        .col = col,
    };
    switch(c) {
        case '+':
            token.type = TT_PLUS;
            break;
        case '-':
            token.type = TT_MINUS;
            break;
        case '*':
            token.type = TT_MULT;
            break;
        case '/':
            token.type = TT_DIV;
            break;
        default:
            ASSERT(false, "unreachable");
    }
    return token;
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
            if(token.type == TT_NONE) {
                PRINT_ERROR(token.loc, "invalid token: %.*s", (int)word.count, word.data);            
            };
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
                if(view.len > 1 && *view.data == '\\') {
                    DA_APPEND(&word, *view.data);                                        
                    view = view_chop_left(view);
                    if(!is_valid_escape(*view.data)) {
                        PRINT_ERROR(token.loc, "unexpected escape character: %c", *view.data);
                    }
                }
                DA_APPEND(&word, *view.data);
                view = view_chop_left(view);
            }
            if(view.len == 0 && *view.data != '"') {
                PRINT_ERROR(token.loc, "expected closing %c quote", '"');                            
            };
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
        } else if(is_operator(*view.data)) {
            Token token = create_operator_token(row, view.data-start+1, *view.data);
            DA_APPEND(&tokens, token);
        } else if(*view.data == '\n') {
            row++;
        }
        view = view_chop_left(view);
    }
    return tokens;
}
    
void token_consume(Token_Arr *tokens) {
    assert(tokens->count != 0);
    tokens->count--;
    tokens->data++;
}

Token token_peek(Token_Arr *tokens, size_t peek_by) {
    if(tokens->count < peek_by) {
        PRINT_ERROR((Location){0}, "error: token count was %zu but expected %zu\n", tokens->count, peek_by);    
    }
    return tokens->data[peek_by];
}

void expect_token(Token_Arr *tokens, Token_Type type) {
    token_consume(tokens);
    if(tokens->data[0].type != type) {
        PRINT_ERROR(tokens->data[0].loc, "expected type: %s, but found type %s\n", 
                    token_types[type], token_types[tokens->data[0].type]);
    };
}

Node *create_node(Node_Type type) {
    Node *node = custom_realloc(NULL, sizeof(Node));
    node->type = type;
    return node;
}

Operator create_operator(Location loc, Token_Type type) {
    Operator op = {0};
    switch(type) {
        case TT_PLUS:
            op.type = OP_PLUS;
            op.precedence = PREC_1;
            break;        
        case TT_MINUS:
            op.type = OP_MINUS;
            op.precedence = PREC_1;
            break;        
        case TT_MULT:
            op.type = OP_MULT;
            op.precedence = PREC_2;
            break;        
        case TT_DIV:
            op.type = OP_DIV;
            op.precedence = PREC_2;
            break;
        default:
            PRINT_ERROR(loc, "expected operator but found: %s\n", token_types[type]);
    }
    return op;
}

bool token_is_op(Token token) {
    switch(token.type) {
        case TT_PLUS:
        case TT_MINUS:
        case TT_MULT:
        case TT_DIV:
            return true;    
        default:
            return false;
    }
}
    
Expr *parse_expr(Token_Arr *tokens) {
    Expr *expr = custom_realloc(NULL, sizeof(Expr));
    Token op_token = token_peek(tokens, 1);
    if(!token_is_op(op_token)) {
        expr->type = EXPR_INT;
        expr->value.integer = tokens->data[0].value.integer;
    } else {
        expr->type = EXPR_BIN;
        expr->value.bin.op = create_operator(tokens->data[1].loc, op_token.type);
        expr->value.bin.lhs = custom_realloc(NULL, sizeof(Expr));
        expr->value.bin.lhs->value.integer = tokens->data[0].value.integer;
        token_consume(tokens);
        token_consume(tokens);        
        expr->value.bin.rhs = parse_expr(tokens);
        //printf("lhs: %d, rhs: %d\n", expr->value.bin.lhs->value.integer, expr->value.bin.rhs->value.integer);     
        expr->value.bin.lhs->type = EXPR_INT;
    }
    return expr;
}
        
Node parse_native_node(Token_Arr *tokens, Token_Type type, int native_value) {
    Node node = {.type = TYPE_NATIVE, .loc = tokens->data[0].loc};            
    expect_token(tokens, type);        
    Native_Call call = {0};
    Arg arg = {0};
    switch(type) {
        case TT_INT: {
            Expr *expr = parse_expr(tokens);
            arg = (Arg){.type=ARG_EXPR, .value.expr=expr};
            //arg = (Arg){.type=VAR_INT, .value.integer=tokens->data[0].value.integer};        
        } break;
        case TT_STRING: {
            arg = (Arg){.type=ARG_STRING, .value.string=tokens->data[0].value.string};                
        } break;
        default:
            PRINT_ERROR(tokens->data[0].loc, "unexpected type: %s\n", token_types[type]);
    }
    DA_APPEND(&call.args, arg);
    call.type = native_value;
    node.value.native = call;
    return node;
}
    
Nodes parse(Token_Arr tokens) {
    Nodes root = {0};
    while(tokens.count > 0) {
        switch(tokens.data[0].type) {
            case TT_WRITE: {
                Node node = parse_native_node(&tokens, TT_STRING, NATIVE_WRITE);            
                DA_APPEND(&root, node);
            } break;
            case TT_EXIT: {
                Node node = parse_native_node(&tokens, TT_INT, NATIVE_EXIT);
                DA_APPEND(&root, node);
            } break;
            case TT_STRING:
            case TT_INT:            
            case TT_PLUS:
            case TT_MINUS:
            case TT_MULT:
            case TT_DIV:
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
    char *output = custom_realloc(NULL, sizeof(char)*filename_s);
    strncpy(output, filename, filename_s);
    strip_off_dot(output);
    char *output_filename = custom_realloc(NULL, sizeof(char)*strlen(output)+sizeof(".tasm"));
    sprintf(output_filename, "%s.tasm", output);
    return output_filename;
}
    
char *op_types[] = {"add", "sub", "mul", "div"};

void print_expr(Expr *expr) {
    if(expr->type == EXPR_INT) {
        printf("int: %d\n", expr->value.integer);
    } else {
        print_expr(expr->value.bin.lhs);
        print_expr(expr->value.bin.rhs);        
    }
}

void compile_expr(FILE *file, Expr *expr) {
    //print_expr(expr);
    switch(expr->type) {
        case EXPR_BIN:
            compile_expr(file, expr->value.bin.lhs);
            compile_expr(file, expr->value.bin.rhs);
            fprintf(file, "%s\n", op_types[expr->value.bin.op.type]);                    
            break;
        case EXPR_INT:
            fprintf(file, "push %d\n", expr->value.integer);        
            break;
        default:
            ASSERT(false, "UNREACHABLE");
    }       
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
                        if(node->value.native.args.data[0].type != ARG_STRING) {
                            PRINT_ERROR(node->loc, "expected type string, but found type %s", node_types[node->value.native.args.data[0].type]);
                        };
                        fprintf(file, "; write\n");
                        fprintf(file, "push_str \""View_Print"\"\n", View_Arg(node->value.native.args.data[0].value.string));
                        fprintf(file, "push %d\n", STDOUT);
                        fprintf(file, "native %d\n", node->value.native.type);
                        break;
                    case NATIVE_EXIT:
                        ASSERT(node->value.native.args.count == 1, "too many arguments");
                        if(node->value.native.args.data[0].type != ARG_EXPR) {
                            PRINT_ERROR(node->loc, "expected type int, but found type %s", node_types[node->value.native.args.data[0].type]);
                        };
                        fprintf(file, "; exit\n");                
                        compile_expr(file, node->value.native.args.data[0].value.expr);
                        //fprintf(file, "push %d\n", node->value.native.args.data[0].value.integer);
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
    fprintf(file, "; exit\n");                
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
