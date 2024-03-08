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
        fprintf(stderr, "%zu:%zu: error: "str"\n", loc.row, loc.col, __VA_ARGS__);  \
        exit(1);                                                                   \
    } while(0)

void *custom_realloc(void *ptr, size_t size) {
    void *new_ptr = realloc(ptr, size);
    ASSERT(new_ptr != NULL, "buy more ram");
    ptr = new_ptr;
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
    TT_IDENT,
    TT_COLON,
    TT_O_PAREN,
    TT_C_PAREN,
    TT_COMMA,
    TT_EQ,
    TT_DOUBLE_EQ,
    TT_NOT_EQ,
    TT_GREATER_EQ,
    TT_LESS_EQ,
    TT_GREATER,
    TT_LESS,
    TT_PLUS,
    TT_MINUS,
    TT_MULT,
    TT_DIV,
    TT_STRING,
    TT_INT,
    TT_TYPE,
    TT_IF,
    TT_ELSE,
    TT_WHILE,
    TT_THEN,
    TT_RET,
    TT_END,
    TT_COUNT,
} Token_Type;
    
char *token_types[TT_COUNT] = {"none", "write", "exit", "ident", 
                               ":", "(", ")", ",", "=", "==", "!=", ">=", "<=", ">", "<", "+", "-", "*", "/", 
                               "string", "integer", "type", "if", "else", "while", "then", "return", "end"};
    
typedef struct {
    size_t row;
    size_t col;
    char *filename;
} Location;

typedef enum {
    TYPE_INT,
    DATA_COUNT,
} Type_Type;

String_View data_types[DATA_COUNT] = {
    {.data="int", .len=3},
};    

typedef union {
    String_View string;
    String_View ident;
    int integer;
    Type_Type type;
} Token_Value;

typedef struct {
    Token_Value value; 
    Token_Type type;
    Location loc;
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
    OP_EQ,
    OP_NOT_EQ,
    OP_GREATER_EQ,
    OP_LESS_EQ,
    OP_GREATER,
    OP_LESS,
} Operator_Type;
    
typedef enum {
    PREC_0 = 0,
    PREC_1,
    PREC_2,
    PREC_COUNT,
} Precedence;
   
typedef struct {
    Operator_Type type;
    Precedence precedence;
} Operator;

struct Expr;    
struct Node;

typedef struct {
    struct Expr *lhs;
    struct Expr *rhs;
    Operator op;
} Bin_Expr;
    
typedef struct {
    struct Expr **data;
    size_t count;
    size_t capacity;
} Exprs;

typedef struct {
    String_View name;
    Exprs args;    
} Func_Call;

typedef enum {
    EXPR_BIN,    
    EXPR_INT,
    EXPR_VAR,
    EXPR_FUNCALL,
} Expr_Type;
    
typedef union {
    Bin_Expr bin;
    int integer;
    String_View variable;
    Func_Call func_call;
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
    
typedef struct {
    struct Node *data;
    size_t count;
    size_t capacity;
} Nodes;

typedef struct {
    String_View name;
    Nodes args;
    Type_Type type;
    Nodes body;
    size_t label;
} Func_Dec;
    
// TODO: rename TYPE_* to NODE_*
typedef enum {
    TYPE_ROOT = 0,
    TYPE_NATIVE,
    TYPE_EXPR,
    TYPE_VAR_DEC,
    TYPE_VAR_REASSIGN,
    TYPE_IF,
    TYPE_ELSE,
    TYPE_WHILE,
    TYPE_THEN,
    TYPE_FUNC_DEC,
    TYPE_FUNC_CALL,
    TYPE_RET,
    TYPE_END,
    TYPE_COUNT,
} Node_Type;
    
char *node_types[TYPE_COUNT] = {"root", "native", "expr", "var_dec", "var_reassign",
                                "if", "else", "while", "then", "func_dec", "func_call", "return", "end"};

typedef struct {
    String_View name;
    Type_Type type;
    Expr *value;
    size_t stack_pos;
} Variable;
    
typedef struct {
    String_View name;
    Nodes args;
    Type_Type type;
} Function;

typedef struct {
    Function *data;
    size_t count;
    size_t capacity;
} Functions;
    
typedef struct {
    Variable *data;
    size_t count;
    size_t capacity;
} Variables;
    
typedef struct {
    size_t label1;
    size_t label2;
} Else_Label;
    
typedef struct {
    size_t num;
    String_View function;
} Label;

typedef union {
    Native_Call native;
    Expr *expr;
    Expr *conditional;
    Variable var;
    Label label;
    Else_Label el;
    Func_Dec func_dec;
    Func_Call func_call;
} Node_Value;

typedef struct Node {
    Node_Type type;
    Node_Value value;
    Location loc;
} Node;
    
typedef struct {
    size_t *data;
    size_t count;
    size_t capacity;
} Size_Stack;

typedef enum {
    BLOCK_IF,
    BLOCK_ELSE,
    BLOCK_WHILE,
    BLOCK_FUNC,
} Block_Type;
    
typedef struct {
    Block_Type *data;
    size_t count;
    size_t capacity;    
} Block_Stack;

typedef struct {
    Variables vars;
    Functions functions;
    size_t stack_s;
    Size_Stack scope_stack;
    Size_Stack ret_stack;
    size_t while_label;
    Size_Stack while_labels;
    Block_Stack block_stack;    
} Program_State;
    
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
    } else if(strncmp(str, "if", str_s) == 0) {
        return TT_IF;
    } else if(strncmp(str, "else", str_s) == 0) {
        return TT_ELSE;  
    } else if (strncmp(str, "while", str_s) == 0) {
        return TT_WHILE;  
    } else if(strncmp(str, "then", str_s) == 0) {
        return TT_THEN;
    } else if(strncmp(str, "return", str_s) == 0) {
        return TT_RET;
    } else if(strncmp(str, "end", str_s) == 0) {
        return TT_END;        
    }
    return TT_NONE;
}

Token handle_data_type(Token token, String_View str) {
    for(size_t i = 0; i < DATA_COUNT; i++) {
        if(view_cmp(str, data_types[i])) {
            token.type = TT_TYPE;
            token.value.type = (Type_Type)i;
        }
    }
    return token;
}
    
bool is_operator(String_View view) {
    switch(*view.data) {
        case '+':
        case '-':
        case '*':
        case '/':
        case '>':
        case '<':
            return true;        
        case '=':
            if(view.len > 1 && view.data[1] == '=') return true;        
            return false;
        case '!':
            if(view.len > 1 && view.data[1] == '=') return true;        
            return false;
        default:
            return false;
    }
}
    
Token create_operator_token(size_t row, size_t col, String_View *view) {
    Token token = {0};
    token.loc = (Location){
        .row = row,
        .col = col,
    };
    switch(*view->data) {
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
        case '=':
            if(view->len > 1 && view->data[1] == '=') {
                *view = view_chop_left(*view);
                token.type = TT_DOUBLE_EQ;        
            }
            break;
        case '!':
            if(view->len > 1 && view->data[1] == '=') {
                *view = view_chop_left(*view);
                token.type = TT_NOT_EQ;        
            }
            break;
        case '>':
            if(view->len > 1 && view->data[1] == '=') {
                *view = view_chop_left(*view);
                token.type = TT_GREATER_EQ;        
            } else {
                *view = view_chop_left(*view);
                token.type = TT_GREATER;        
            }
            break;
        case '<':
            if(view->len > 1 && view->data[1] == '=') {
                *view = view_chop_left(*view);
                token.type = TT_LESS_EQ;        
            } else {
                *view = view_chop_left(*view);
                token.type = TT_LESS;
            }
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
                .col = view.data-start,
            };
            Dynamic_Str word = {0};    
            DA_APPEND(&word, *view.data);                    
            view = view_chop_left(view);
            while(view.len > 0 && isword(*view.data)) {
                DA_APPEND(&word, *view.data);            
                view = view_chop_left(view);    
            }
            view.data--;
            view.len++;
            token.type = get_token_type(word.data, word.count);
            token = handle_data_type(token, view_create(word.data, word.count));
            if(token.type == TT_NONE) {
                token.type = TT_IDENT;
                char *ident = custom_realloc(NULL, sizeof(char)*word.count);
                strncpy(ident, word.data, word.count);
                token.value.ident = view_create(ident, word.count);
            };
            DA_APPEND(&tokens, token);     
            free(word.data);
        } else if(*view.data == '"') {
            Token token = {0};
            token.loc = (Location){
                .row = row,
                .col = view.data-start,
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
                .col = view.data-start,
            };
        
            Dynamic_Str num = {0};
            while(view.len > 0 && isdigit(*view.data)) {
                DA_APPEND(&num, *view.data);
                view = view_chop_left(view);
            }
            view.data--;
            view.len++;
                
            DA_APPEND(&num, '\0');
            token.type = TT_INT;
            token.value.integer = atoi(num.data);
            DA_APPEND(&tokens, token);                        
        } else if(is_operator(view)) {
            Token token = create_operator_token(row, view.data-start, &view);
            DA_APPEND(&tokens, token);                                        
        } else if(*view.data == ':') {
            Token token = {.loc = (Location) {.row=row, .col=view.data-start}, .type = TT_COLON};
            DA_APPEND(&tokens, token);                                                    
        } else if(*view.data == '=') {
            Token token = {.loc = (Location) {.row=row, .col=view.data-start}, .type = TT_EQ};   
            DA_APPEND(&tokens, token);                                                    
        } else if(*view.data == '(') {
            Token token = {.loc = (Location) {.row=row, .col=view.data-start}, .type = TT_O_PAREN};   
            DA_APPEND(&tokens, token);                                                    
        } else if(*view.data == ')') {
            Token token = {.loc = (Location) {.row=row, .col=view.data-start}, .type = TT_C_PAREN};   
            DA_APPEND(&tokens, token);                                                    
        } else if(*view.data == ',') {
            Token token = {.loc = (Location) {.row=row, .col=view.data-start}, .type = TT_COMMA};   
            DA_APPEND(&tokens, token);                                                    
                    
        } else if(*view.data == '\n') {
            row++;
            start = view.data;
        }
        view = view_chop_left(view);
    }
    return tokens;
}
    
Token token_consume(Token_Arr *tokens) {
    ASSERT(tokens->count != 0, "out of tokens");
    tokens->count--;
    return *tokens->data++;
}

Token token_peek(Token_Arr *tokens, size_t peek_by) {
    if(tokens->count <= peek_by) {
        return (Token){0};
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
    
Precedence op_get_prec(Token_Type type) {
    switch(type) {
        case TT_PLUS:
        case TT_MINUS:
        case TT_DOUBLE_EQ:
        case TT_NOT_EQ:
        case TT_GREATER_EQ:
        case TT_LESS_EQ:
        case TT_GREATER:
        case TT_LESS:
            return PREC_1;
        case TT_MULT:
        case TT_DIV:
            return PREC_2;
            break;
        default:
            return PREC_0;
    }
}

Operator create_operator(Token_Type type) {
    Operator op = {0};
    op.precedence = op_get_prec(type);
    switch(type) {
        case TT_PLUS:
            op.type = OP_PLUS;
            break;        
        case TT_MINUS:
            op.type = OP_MINUS;
            break;        
        case TT_MULT:
            op.type = OP_MULT;
            break;        
        case TT_DIV:
            op.type = OP_DIV;
            break;
        case TT_DOUBLE_EQ:
            op.type = OP_EQ;
            break;
        case TT_NOT_EQ:
            op.type = OP_NOT_EQ;
            break;
        case TT_GREATER_EQ:
            op.type = OP_GREATER_EQ;
            break;
        case TT_LESS_EQ:
            op.type = OP_LESS_EQ;
            break;
        case TT_GREATER:
            op.type = OP_GREATER;
            break;
        case TT_LESS:
            op.type = OP_LESS;
            break;
        default:
            return (Operator){0};
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

void print_expr(Expr *expr) {
    if(expr->type == EXPR_INT) {
        printf("int: %d\n", expr->value.integer);
    } else {
        print_expr(expr->value.bin.lhs);
        print_expr(expr->value.bin.rhs);        
    }
}
    
void gen_push(Program_State *state, FILE *file, int value) {
    fprintf(file, "push %d\n", value);
    state->stack_s++;   
}
    
void gen_pop(Program_State *state, FILE *file) {
    fprintf(file, "pop\n");
    state->stack_s--;   
}

void gen_push_str(Program_State *state, FILE *file, String_View value) {
    fprintf(file, "push_str \""View_Print"\"\n", View_Arg(value));
    state->stack_s++;
}
    
void gen_indup(Program_State *state, FILE *file, size_t value) {
    fprintf(file, "indup %zu\n", value);
    state->stack_s++;
}
    
void gen_inswap(FILE *file, size_t value) {
    fprintf(file, "inswap %zu\n", value);
}
    
void gen_zjmp(Program_State *state, FILE *file, size_t label) {
    fprintf(file, "zjmp label%zu\n", label);
    state->stack_s--;
}
    
void gen_jmp(FILE *file, size_t label) {
    fprintf(file, "jmp label%zu\n", label);
}
    
void gen_while_jmp(FILE *file, size_t label) {
    fprintf(file, "jmp while%zu\n", label);
}
    
void gen_label(FILE *file, size_t label) {
    fprintf(file, "label%zu:\n", label);
}
    
void gen_func_label(FILE *file, String_View label) {
    fprintf(file, View_Print":\n", View_Arg(label));
}
    
void gen_func_call(FILE *file, String_View label) {
    fprintf(file, "call "View_Print"\n", View_Arg(label));
}
    
void gen_while_label(FILE *file, size_t label) {
     fprintf(file, "while%zu:\n", label);   
}
    
Expr *parse_expr(Token_Arr *tokens);
    
// parse primary takes a primary, currently only supports integers
// but in the future, could be identifiers, etc
Expr *parse_primary(Token_Arr *tokens) {
    Token token = token_consume(tokens);
    if(token.type != TT_INT && token.type != TT_IDENT) {
        PRINT_ERROR(token.loc, "expected %s but found %s", token_types[TT_INT], token_types[token.type]);
    }
    Expr *expr = custom_realloc(NULL, sizeof(Expr));   
    if(token.type == TT_INT) {
        *expr = (Expr){
            .type = EXPR_INT,
            .value.integer = token.value.integer,
        };
    } else if(token.type == TT_IDENT) {
        if(token_peek(tokens, 0).type == TT_O_PAREN) {
            *expr = (Expr){
                .type = EXPR_FUNCALL,
                .value.func_call.name = token.value.ident,
            };
            while(tokens->count > 0 && token_consume(tokens).type != TT_C_PAREN) {
                Expr *arg = parse_expr(tokens);
                DA_APPEND(&expr->value.func_call.args, arg);
            }
        } else {
            *expr = (Expr){
                .type = EXPR_VAR,
                .value.variable = token.value.ident,
            };
        }
    }
    return expr;
}

    
Expr *parse_expr_1(Token_Arr *tokens, Expr *lhs, Precedence min_precedence) {
    Token lookahead = token_peek(tokens, 0);
    // make sure it's an operator
    while(op_get_prec(lookahead.type) >= min_precedence) {
        Operator op = create_operator(lookahead.type);    
        if(tokens->count > 0) {
            token_consume(tokens);
            Expr *rhs = parse_primary(tokens);
            lookahead = token_peek(tokens, 0);
            while(op_get_prec(lookahead.type) > op.precedence) {
                rhs = parse_expr_1(tokens, rhs, op.precedence+1);
                lookahead = token_peek(tokens, 0);
            }
            // allocate new lhs node to ensure old lhs does not point
            // at itself, getting stuck in a loop
            Expr *new_lhs = custom_realloc(NULL, sizeof(Expr));
            *new_lhs = (Expr) {
                .type = EXPR_BIN,
                .value.bin.lhs = lhs,
                .value.bin.rhs = rhs,
                .value.bin.op = op,
            };
            lhs = new_lhs;
        }
    }
    return lhs;
}
    
Expr *parse_expr(Token_Arr *tokens) {
    return parse_expr_1(tokens, parse_primary(tokens), 1);
}

Node parse_native_node(Token_Arr *tokens, Token_Type type, int native_value) {
    Node node = {.type = TYPE_NATIVE, .loc = tokens->data[0].loc};            
    //expect_token(tokens, type);        
    token_consume(tokens);
    Token token = tokens->data[0];
    Native_Call call = {0};
    Arg arg = {0};
    switch(type) {
        case TT_INT: {
            if(token.type != TT_INT && token.type != TT_IDENT) PRINT_ERROR(token.loc, "expected int or ident but found %s\n", token_types[token.type]);
            arg = (Arg){.type=ARG_EXPR, .value.expr=parse_expr(tokens)};
        } break;
        case TT_STRING: {
            if(token.type != TT_STRING) PRINT_ERROR(token.loc, "expected string but found %s\n", token_types[token.type]);        
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
    
Node parse_var_dec(Token_Arr *tokens) {
    Node node = {0};
    node.type = TYPE_VAR_DEC;
    node.value.var.name = tokens->data[0].value.ident;
    expect_token(tokens, TT_COLON);
    expect_token(tokens, TT_TYPE);                
    node.value.var.type = tokens->data[0].value.type;
    return node;
}
    
typedef struct {
    String_View value;
    Block_Type type;
} Block;

typedef struct {
    Block *data;
    size_t count;
    size_t capacity;
} Blocks;
    
typedef struct {
    Nodes nodes;  
    Nodes functions;
} Program;
    
Program parse(Token_Arr tokens, Blocks *block_stack) {
    Nodes root = {0};
    Nodes functions = {0};
    size_t cur_label = 0;
    Size_Stack labels = {0};
    while(tokens.count > 0) {
        switch(tokens.data[0].type) {
            case TT_WRITE: {
                Node node = parse_native_node(&tokens, TT_STRING, NATIVE_WRITE);            
                DA_APPEND(&root, node);
                token_consume(&tokens);
            } break;
            case TT_EXIT: {
                Node node = parse_native_node(&tokens, TT_INT, NATIVE_EXIT);
                DA_APPEND(&root, node);
            } break;
            case TT_IDENT: {
                Node node = {0};
                Token token = token_peek(&tokens, 1);
                if(token.type == TT_COLON) {
                    node = parse_var_dec(&tokens);
                    expect_token(&tokens, TT_EQ);                
                    token_consume(&tokens);
                    
                    node.value.var.value = parse_expr(&tokens);
                } else if(token.type == TT_EQ) {
                    node.type = TYPE_VAR_REASSIGN;
                    node.value.var.name = tokens.data[0].value.ident;
                        
                    token_consume(&tokens);
                    token_consume(&tokens);                    
                    
                    node.value.var.value = parse_expr(&tokens);
                } else if(token.type == TT_O_PAREN) {
                    size_t i = 1;
                    while(i < tokens.count+1 && token_peek(&tokens, i).type != TT_C_PAREN) i++;
                    if(i == tokens.count) {
                        PRINT_ERROR(tokens.data[0].loc, "expected %s\n", token_types[TT_C_PAREN]);
                    }
                    Token token = token_peek(&tokens, i+1);
                    if(token.type == TT_COLON) {
                        // function dec
                        node.type = TYPE_FUNC_DEC;
                        node.value.func_dec.name = tokens.data[0].value.ident;                                        
                        Block block = {.type = BLOCK_FUNC, .value = node.value.func_dec.name};
                        DA_APPEND(block_stack, block);                        
                        token_consume(&tokens);                        
                        while(tokens.count > 0 && token_consume(&tokens).type != TT_C_PAREN) {
                            Node arg = parse_var_dec(&tokens);
                            DA_APPEND(&node.value.func_dec.args, arg);
                            token_consume(&tokens);
                        }
                        token_consume(&tokens);                                                                        
                        node.value.func_dec.type = tokens.data[0].value.type;
                        token_consume(&tokens);                                                                        
                        node.value.func_dec.label = cur_label;
                        DA_APPEND(&labels, cur_label++);
                    } else {
                        // function call
                        node.type = TYPE_FUNC_CALL;
                        node.value.func_call.name = tokens.data[0].value.ident;                                        
                        token_consume(&tokens);                                                
                        while(tokens.count > 0 && token_consume(&tokens).type != TT_C_PAREN) {
                            Expr *arg = parse_expr(&tokens);
                            DA_APPEND(&node.value.func_call.args, arg);
                        }
                    }
                } else {
                    PRINT_ERROR(token.loc, "unexpected token %s\n", token_types[token.type]);
                }
                DA_APPEND(&root, node);                
            } break;       
            case TT_RET: {
                Node node = {.type = TYPE_RET};
                token_consume(&tokens);
                node.value.expr = parse_expr(&tokens);
                DA_APPEND(&root, node);                
            } break;
            case TT_IF: {
                Node node = {.type = TYPE_IF};
                DA_APPEND(block_stack, (Block){.type=BLOCK_IF});                
                token_consume(&tokens);
                node.value.conditional = parse_expr(&tokens);
                DA_APPEND(&root, node);
            } break;
            case TT_ELSE: {
                Node node = {.type = TYPE_ELSE};
                token_consume(&tokens);
                DA_APPEND(block_stack, (Block){.type=BLOCK_ELSE});                                                
                node.value.el.label1 = labels.data[--labels.count];                
                node.value.el.label2 = cur_label;
                DA_APPEND(&labels, cur_label++);
                DA_APPEND(&root, node);                
            } break;
            case TT_WHILE: {
                Node node = {.type = TYPE_WHILE};
                token_consume(&tokens);
                DA_APPEND(block_stack, (Block){.type=BLOCK_WHILE});                                
                node.value.conditional = parse_expr(&tokens);
                DA_APPEND(&root, node);
            } break;
            case TT_THEN: {
                Node node = {.type = TYPE_THEN};
                token_consume(&tokens);                
                node.value.label.num = cur_label;
                DA_APPEND(&labels, cur_label++);
                DA_APPEND(&root, node);                
            } break;
            case TT_END: {
                Node node = {.type = TYPE_END};
                token_consume(&tokens);                                
                if(labels.count == 0) PRINT_ERROR(node.loc, "labels count was: %zu", labels.count);
                node.value.label.num = labels.data[--labels.count];   
                DA_APPEND(&root, node);                
            } break;
            case TT_STRING:
            case TT_INT:            
            case TT_PLUS:
            case TT_COLON:
            case TT_EQ:
            case TT_DOUBLE_EQ:
            case TT_NOT_EQ:
            case TT_O_PAREN:
            case TT_C_PAREN:
            case TT_COMMA:
            case TT_GREATER_EQ:
            case TT_LESS_EQ:
            case TT_GREATER:
            case TT_LESS:
            case TT_TYPE:
            case TT_MINUS:
            case TT_MULT:
            case TT_DIV:
            case TT_NONE:
            case TT_COUNT:
                printf("type: %s %d\n", token_types[tokens.data[0].type], tokens.data[0].value.integer);
                ASSERT(false, "unreachable");
        }
    }
    Program program = {0};
    program.nodes = root;
    program.functions = functions;
    return program;
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
    
char *op_types[] = {"add", "sub", "mul", "div", "cmpe", "cmpne", "cmpge", "cmple", "cmpg", "cmpl"};

Function *get_func(Functions functions, String_View name) {
    for(size_t i = 0; i < functions.count; i++) {
        if(view_cmp(functions.data[i].name, name)) {
            return &functions.data[i];
        }
    }
    return false;
}

int get_variable_location(Program_State *state, String_View name) {
    for(size_t i = 0; i < state->vars.count; i++) {
        if(view_cmp(state->vars.data[i].name, name)) {
            return state->vars.data[i].stack_pos;
        }
    }
    return -1;
}

void gen_expr(Program_State *state, FILE *file, Expr *expr) {
    switch(expr->type) {
        case EXPR_BIN:
            gen_expr(state, file, expr->value.bin.lhs);
            gen_expr(state, file, expr->value.bin.rhs);
            fprintf(file, "%s\n", op_types[expr->value.bin.op.type]);                    
            state->stack_s--;
            break;
        case EXPR_INT:
            gen_push(state, file, expr->value.integer);        
            break;
        case EXPR_VAR: {
            int index = get_variable_location(state, expr->value.variable);
            if(index == -1) {
                PRINT_ERROR((Location){0}, "variable "View_Print" referenced before assignment", View_Arg(expr->value.variable));
            }
            gen_indup(state, file, state->stack_s-index); 
        } break;
        case EXPR_FUNCALL: {
            Function *function = get_func(state->functions, expr->value.func_call.name);
            if(!function) { 
                PRINT_ERROR((Location){0}, "function "View_Print" referenced before assignment\n", View_Arg(expr->value.func_call.name));
            }
            if(function->args.count != expr->value.func_call.args.count) {
                PRINT_ERROR((Location){0}, "args count do not match for function "View_Print"\n", View_Arg(function->name));
            }
            for(size_t i = 0; i < expr->value.func_call.args.count; i++) {
                gen_expr(state, file, expr->value.func_call.args.data[i]);
            }
            gen_func_call(file, expr->value.func_call.name);
        } break;
        default:
            ASSERT(false, "UNREACHABLE");
    }       
}

void scope_end(Program_State *state, FILE *file) {
    ASSERT(state->scope_stack.count > 0, "scope stack count == 0");
    size_t target = state->scope_stack.data[state->scope_stack.count-1];
    while(state->stack_s > target) {
        gen_pop(state, file);
    }
    while(state->vars.count > 0 && state->vars.data[state->vars.count-1].stack_pos > state->stack_s) {
        state->vars.count--;
    }
}

void ret_scope_end(Program_State *state, FILE *file) {
    ASSERT(state->ret_stack.count > 0, "scope stack count == 0");
    size_t target = state->ret_stack.data[state->ret_stack.count-1];
    if(state->stack_s > 0) state->stack_s--;
    while(state->stack_s > target) {
        gen_pop(state, file);
    }
}
    
void gen_program(Program_State *state, Nodes nodes, FILE *file) {
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
                        gen_push_str(state, file, node->value.native.args.data[0].value.string);
                        gen_push(state, file, STDOUT);
                        fprintf(file, "native %d\n", node->value.native.type);
                        state->stack_s -= 2;
                        break;
                    case NATIVE_EXIT:
                        ASSERT(node->value.native.args.count == 1, "too many arguments");
                        if(node->value.native.args.data[0].type != ARG_EXPR) {
                            PRINT_ERROR(node->loc, "expected type int, but found type %s", node_types[node->value.native.args.data[0].type]);
                        };
                        fprintf(file, "; exit\n");                
                        fprintf(file, "; expr\n");                                
                        gen_expr(state, file, node->value.native.args.data[0].value.expr);
                        fprintf(file, "native %d\n", node->value.native.type);
                        state->stack_s--;
                        break;           
                    default:
                        ASSERT(false, "unreachable");
                }
                break;
            case TYPE_VAR_DEC: {
                fprintf(file, "; var declaration\n");                                                        
                fprintf(file, "; expr\n");                                            
                gen_expr(state, file, node->value.var.value);
                node->value.var.stack_pos = state->stack_s; 
                DA_APPEND(&state->vars, node->value.var);    
            } break;
            case TYPE_VAR_REASSIGN: {
                fprintf(file, "; var reassign\n");                                            
                fprintf(file, "; expr\n");                                                                
                gen_expr(state, file, node->value.var.value);
                int index = get_variable_location(state, node->value.var.name);
                if(index == -1) {
                    PRINT_ERROR((Location){0}, "variable "View_Print" referenced before assignment", View_Arg(node->value.var.name));
                }
                gen_inswap(file, state->stack_s-index);    
                gen_pop(state, file);
                DA_APPEND(&state->vars, node->value.var);    
            } break;
            case TYPE_FUNC_DEC: {
                Function function = {0};
                function.type = node->value.func_dec.type;
                function.args = node->value.func_dec.args;
                function.name = node->value.func_dec.name;
                DA_APPEND(&state->functions, function);
                DA_APPEND(&state->block_stack, BLOCK_FUNC);                
                DA_APPEND(&state->ret_stack, state->stack_s);                
                DA_APPEND(&state->scope_stack, state->stack_s);                
                for(size_t i = 0; i < function.args.count; i++) {
                    Variable var = {0};
                    var.stack_pos = ++state->stack_s;
                    var.name = function.args.data[i].value.var.name;
                    DA_APPEND(&state->vars, var);    
                }
                gen_jmp(file, node->value.func_dec.label);                                
                gen_func_label(file, function.name);
            } break;
            case TYPE_FUNC_CALL: {
                Function *function = get_func(state->functions, node->value.func_call.name);
                if(!function) { 
                    PRINT_ERROR((Location){0}, "function "View_Print" referenced before assignment\n", View_Arg(node->value.func_call.name));
                }
                if(function->args.count != node->value.func_call.args.count) {
                    PRINT_ERROR((Location){0}, "args count do not match for function "View_Print"\n", View_Arg(function->name));
                }
                for(size_t i = 0; i < node->value.func_call.args.count; i++) {
                    gen_expr(state, file, node->value.func_call.args.data[i]);
                }
                gen_func_call(file, node->value.func_call.name);
                state->stack_s -= node->value.func_call.args.count;
            } break;
            case TYPE_RET: {
                fprintf(file, "; return\n");
                size_t pos = state->scope_stack.data[state->ret_stack.count-1] + 1;
                gen_expr(state, file, node->value.expr);
                ASSERT(pos <= state->stack_s, "pos is too great");
                gen_inswap(file, state->stack_s-pos);
                size_t pre_stack_s = state->stack_s;
                ret_scope_end(state, file);
                state->stack_s = pre_stack_s;
                fprintf(file, "ret\n");
            } break;
            case TYPE_IF: {
                fprintf(file, "; if statement\n");                                                                                
                fprintf(file, "; expr\n");                                                                            
                DA_APPEND(&state->block_stack, BLOCK_IF);
                gen_expr(state, file, node->value.conditional);
            } break;
            case TYPE_ELSE: {
                fprintf(file, "; else statement\n");                                                                                
                if(state->block_stack.data[--state->block_stack.count] == BLOCK_IF) {
                    gen_jmp(file, node->value.el.label2);
                } else {
                    PRINT_ERROR((Location){0}, "expected if but found %d\n", state->block_stack.data[state->block_stack.count]);
                }
                DA_APPEND(&state->block_stack, BLOCK_ELSE);                
                gen_label(file, node->value.el.label1);
            } break;
            case TYPE_WHILE: {
                fprintf(file, "; while loop\n");                                                                                
                fprintf(file, "; expr\n");                                                                            
                DA_APPEND(&state->block_stack, BLOCK_WHILE);
                DA_APPEND(&state->while_labels, state->while_label);
                gen_while_label(file, state->while_label++);
                gen_expr(state, file, node->value.conditional);
            } break;
            case TYPE_THEN: {
                fprintf(file, "; then\n");                                                                                
                gen_zjmp(state, file, node->value.label.num);            
                DA_APPEND(&state->scope_stack, state->stack_s);
            } break;
            case TYPE_END: {
                fprintf(file, "; end\n");                                                                                                        
                if(state->block_stack.count == 0) {
                    PRINT_ERROR((Location){0}, "error: block stack: %zu\n", state->block_stack.count);
                }
                scope_end(state, file);                    
                state->scope_stack.count--;
                Block_Type block = state->block_stack.data[--state->block_stack.count];
                if(block == BLOCK_WHILE) {
                    fprintf(file, "; end while\n");                                                                                                                
                    gen_while_jmp(file, state->while_labels.data[--state->while_labels.count]);
                } else if(block == BLOCK_FUNC) {
                    fprintf(file, "; end func\n");                                                                                                                
                    fprintf(file, "ret\n");                                                                                                                                    
                }
                gen_label(file, node->value.label.num);
            } break;
            default:
                break;
        }    
    }

}
    
void generate(Program_State *state, Program *program, char *filename) {
    char *output = append_tasm_ext(filename);
    FILE *file = fopen(output, "w");
    gen_program(state, program->nodes, file);
    fprintf(file, "; exit\n");                
    gen_push(state, file, 0);
    fprintf(file, "native 60\n");
    state->stack_s--;
    //gen_program(state, program->functions, program->functions, file);        
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        usage(argv[0]);
    }
    char *filename = argv[1];
    String_View view = read_file_to_view(filename);
    Token_Arr tokens = lex(view);
    //print_token_arr(tokens);
    Blocks block_stack = {0};
    Program program = parse(tokens, &block_stack);
    Program_State state = {0};
    generate(&state, &program, filename);
}
