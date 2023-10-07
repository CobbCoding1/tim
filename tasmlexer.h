#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TYPE_NONE = -1,
    TYPE_NOP = 0,
    TYPE_PUSH,
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
    TYPE_CMPE,
    TYPE_CMPNE,
    TYPE_CMPG,
    TYPE_CMPL,
    TYPE_CMPGE,
    TYPE_CMPLE,
    TYPE_JMP,
    TYPE_ZJMP,
    TYPE_NZJMP,
    TYPE_PRINT,
    TYPE_HALT,
} TokenType;

typedef struct {
    TokenType type;
    char *text;
    int line;
    int character;
} Token;

int lexer();

#endif
