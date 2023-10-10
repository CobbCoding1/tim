#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include "tasmlexer.h"

typedef struct ParseList {
    Token value;
    struct ParseList *next;
} ParseList;

void append(ParseList *head, Token value);
void print_list(ParseList *head);
void generate_list(ParseList *root, Lexer *lexer);
ParseList parser(Lexer lexer);

#endif
