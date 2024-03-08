#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <stdarg.h>

#include "tasmlexer.h"

#include "hashmap.h"

typedef struct ParseList {
    Token value;
    struct ParseList *next;
} ParseList;

void append(ParseList *head, Token value);
void print_list(ParseList *head);
void handle_token_def(Lexer *lexer, Token token, int index, int line_num, struct hashmap_s *hashmap);
void print_syntax_error(Token *current_token, char *type_of_error, char *expected);
int expect_token(Lexer *lexer, int index, int count, ...);
int check_if_register(TokenType type);
void generate_list(ParseList *root, Lexer *lexer, struct hashmap_s *hashmap);
void check_labels(ParseList *head, Lexer *lexer, struct hashmap_s *hashmap);
ParseList parser(Lexer lexer);

#endif
