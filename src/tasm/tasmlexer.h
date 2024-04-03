#ifndef LEXER_H
#define LEXER_H

#define MAX_TOKEN_STACK_SIZE 2048

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

typedef enum {
    TYPE_NONE = -1,
    TYPE_NOP = 0,
    TYPE_PUSH,
    TYPE_PUSH_STR,
    TYPE_MOV,
    TYPE_REF,
    TYPE_DEREF,
    TYPE_ALLOC,
    TYPE_DEALLOC,
    TYPE_WRITE,
    TYPE_READ,
    TYPE_POP,
    TYPE_DUP,
    TYPE_INDUP,
    TYPE_SWAP,
    TYPE_INSWAP,
    TYPE_ADD,
    TYPE_SUB,
    TYPE_MUL,
    TYPE_DIV,
    TYPE_MOD,
    TYPE_ADD_F,
    TYPE_SUB_F,
    TYPE_MUL_F,
    TYPE_DIV_F,
    TYPE_MOD_F,
    TYPE_CMPE,
    TYPE_CMPNE,
    TYPE_CMPG,
    TYPE_CMPL,
    TYPE_CMPGE,
    TYPE_CMPLE,
    TYPE_ITOF,
    TYPE_FTOI,
    TYPE_ITOC,
    TYPE_TOI,
    TYPE_TOF,
    TYPE_TOC,
    TYPE_TOVP,
    TYPE_CALL,
    TYPE_RET,
    TYPE_JMP,
    TYPE_ZJMP,
    TYPE_NZJMP,
    TYPE_PRINT,
    TYPE_NATIVE,
    TYPE_ENTRYPOINT,
    TYPE_SS,
    TYPE_HALT,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_CHAR,
    TYPE_STRING,
    TYPE_NULL,
    TYPE_REGISTER,
    TYPE_LABEL_DEF,
    TYPE_LABEL,
    TYPE_TOP,
    TYPE_COUNT,
} TokenType;

typedef struct {
    TokenType type;
    char *text;
    int line;
    int character;
    char *file_name;
} Token;

typedef struct {
    Token token_stack[MAX_TOKEN_STACK_SIZE];
    int stack_size;
    char *file_name;
} Lexer;

int is_name(char character);
char *open_file(char *file_path, int *length);
void push_token(Lexer *lexer, Token value);
Token pop_token(Lexer *lexer);
char *pretty_token(Token token);
void print_token(Token token);
Token init_token(TokenType type, char *text, int line, int character, char *file_name);
TokenType check_builtin_keywords(char *name);
TokenType check_label_type(char *current, int *current_index);
Token generate_keyword(char *current, int *current_index, int *line, int *character, Lexer lex);
Token generate_num(char *current, int *current_index, int line, int *character, Lexer lex);
char valid_escape_character(char *current, int *current_index);
Token generate_char(char *file_name, char *current, int *current_index, int line, int *character, Lexer lex);
Token generate_string(char *file_name, char *current, int *current_index, int line, int *character, Lexer lex);
Lexer lexer();

#endif
