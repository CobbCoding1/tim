#pragma once
#ifndef DEFS_H
#define DEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "view.h"
#include "arena.h"

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

#define ADA_APPEND(arena, da, item) do {                                                       \
    if ((da)->count >= (da)->capacity) {                                               \
        (da)->capacity = (da)->capacity == 0 ? DATA_START_CAPACITY : (da)->capacity*2; \
        (da)->data = arena_realloc((arena), (da)->data, (da)->count*sizeof(*(da)->data), (da)->capacity*sizeof(*(da)->data));       \
        ASSERT((da)->data != NULL, "outta ram");                               \
    }                                                                                  \
    (da)->data[(da)->count++] = (item);                                               \
} while (0)
	
#define DA_APPEND(da, item) do {                                                       \
    if ((da)->count >= (da)->capacity) {                                               \
        (da)->capacity = (da)->capacity == 0 ? DATA_START_CAPACITY : (da)->capacity*2; \
        (da)->data = custom_realloc((da)->data, (da)->capacity*sizeof(*(da)->data));       \
        ASSERT((da)->data != NULL, "outta ram");                               \
    }                                                                                  \
    (da)->data[(da)->count++] = (item);                                               \
} while (0)
    
#define PRINT_ERROR(loc, ...)                                                 \
    do {                                                                           \
        fprintf(stderr, "%s:%zu:%zu: error: ", loc.filename, loc.row, loc.col);  \
        fprintf(stderr, __VA_ARGS__);    \
        fprintf(stderr, "\n"); \
        exit(1);                                                                   \
    } while(0)
    
typedef enum {
    BUILTIN_ALLOC = 0,
    BUILTIN_DEALLOC,
    BUILTIN_STORE,
    BUILTIN_TOVP,
    BUILTIN_GET,        
} Builtin_Type;
    
typedef struct {
    char *data;
    size_t count;
    size_t capacity;
} Dynamic_Str;
    
typedef enum {
    TYPE_INT,
    TYPE_STR,
    TYPE_VOID,
    TYPE_CHAR,
    TYPE_FLOAT,
    TYPE_PTR,
    DATA_COUNT,
} Type_Type;

typedef struct {
    size_t row;
    size_t col;
    char *filename;
} Location;


typedef enum {
    OP_PLUS,
    OP_MINUS,
    OP_MULT,
    OP_DIV,
    OP_MOD,
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
    Builtin_Type type;
    Exprs value;
    Type_Type return_type;
} Builtin;

typedef struct {
    String_View name;
    Exprs args;    
} Func_Call;
    
typedef struct {
    String_View name;
    struct Expr *index;
    String_View var_name;	
} Array;
    
typedef struct {
    String_View structure;
    String_View var_name;
    Exprs value;
} Field;

typedef enum {
    EXPR_BIN,    
    EXPR_INT,
    EXPR_FLOAT,
    EXPR_STR,
    EXPR_CHAR,
    EXPR_VAR,
    EXPR_FUNCALL,
    EXPR_ARR,
    EXPR_FIELD,
	EXPR_FIELD_ARR,
    EXPR_BUILTIN,
    EXPR_COUNT,
} Expr_Type;

typedef union {
    Bin_Expr bin;
    int integer;
    double floating;
    Field field;
    Array array;
    String_View variable;
    String_View string;
    Func_Call func_call;
    Builtin builtin;
} Expr_Value;

typedef struct Expr {
    Expr_Value value;
    Expr_Type type;   
    Type_Type return_type;
    Location loc;
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
    String_View name;
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
    TYPE_EXPR_STMT,
    TYPE_VAR_DEC,
    TYPE_VAR_REASSIGN,
    TYPE_FIELD_REASSIGN,
    TYPE_IF,
    TYPE_ELSE,
    TYPE_WHILE,
    TYPE_THEN,
    TYPE_FUNC_DEC,
    TYPE_FUNC_CALL,
    TYPE_STRUCT,
    TYPE_ARR_INDEX,
    TYPE_RET,
    TYPE_END,
    TYPE_COUNT,
} Node_Type;
    
typedef struct {
    String_View name;
    String_View struct_name;
	String_View function;	
    Args struct_value;
    Type_Type type;
    Exprs value;
    size_t stack_pos;
    bool is_array;
    bool is_struct;
	bool global;	
    Expr *array_s;
} Variable;
    
typedef struct {
    String_View name;
    Nodes args;
    Type_Type type;
	size_t label;	
} Function;
	
typedef struct {
	Function *data;
	size_t count;
	size_t capacity;
} Functions;
    
typedef struct {
    String_View name;
    Nodes values;
} Struct;

typedef struct {
    Variable *data;
    size_t count;
    size_t capacity;
} Variables;
    
typedef struct {
    String_View name;
    Expr *index;
    Exprs value;
} Array_Index;
    
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
    Expr *expr_stmt;
    Variable var;
    Array_Index array;
    Label label;
    Else_Label el;
    Func_Dec func_dec;
    Func_Call func_call;
    Struct structs;
    Field field;
} Node_Value;

typedef struct Node {
    Node_Type type;
    Node_Value value;
    Location loc;
} Node;

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
    size_t *data;
    size_t count;
    size_t capacity;
} Size_Stack;

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
    Functions functions;
    Nodes structs;
	Nodes vars;
} Program;

void *custom_realloc(void *ptr, size_t size);
    
#endif // DEFS_H
