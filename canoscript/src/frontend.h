#pragma once
#ifndef FRONTEND_H
#define FRONTEND_H

#include "defs.h"

// Lexing
typedef enum {
    TT_NONE = 0,
    TT_WRITE,
    TT_EXIT,
    TT_IDENT,
    TT_COLON,
    TT_O_PAREN,
    TT_C_PAREN,
    TT_O_BRACKET,
    TT_C_BRACKET,
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
    TT_CHAR_LIT,
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
    
bool is_valid_escape(char c);
String_View read_file_to_view(char *filename);
bool isword(char c);
Token_Type get_token_type(String_View str);
Token handle_data_type(Token token, String_View str);
bool is_operator(String_View view);
Token create_operator_token(size_t row, size_t col, String_View *view);
void print_token_arr(Token_Arr arr);
Token_Arr lex(String_View view);
Token token_consume(Token_Arr *tokens);
Token token_peek(Token_Arr *tokens, size_t peek_by);
void expect_token(Token_Arr *tokens, Token_Type type);
Node *create_node(Node_Type type);
Precedence op_get_prec(Token_Type type);
Operator create_operator(Token_Type type);
Expr *parse_expr(Token_Arr *tokens);
Expr *parse_primary(Token_Arr *tokens);
Expr *parse_expr_1(Token_Arr *tokens, Expr *lhs, Precedence min_precedence);
Expr *parse_expr(Token_Arr *tokens);
Node parse_native_node(Token_Arr *tokens, Token_Type type, int native_value);
Node parse_var_dec(Token_Arr *tokens);
Program parse(Token_Arr tokens, Blocks *block_stack);

#endif // FRONTEND_H