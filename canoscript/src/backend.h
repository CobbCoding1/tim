#pragma once
#ifndef BACKEND_H
#define BACKEND_H

#include "defs.h"
#include "tim.h"

typedef struct {
	size_t *data;
	size_t count;	
	size_t capacity;
} Labels;

typedef struct {
    Variables vars;
    Functions functions;
    size_t stack_s;
    Size_Stack scope_stack;
    Size_Stack ret_stack;
    size_t while_label;
    Size_Stack while_labels;
    Block_Stack block_stack;    
    Nodes structs;
	Program program;
	Labels labels;
	Machine machine;
} Program_State;
    
void gen_push(Program_State *state, FILE *file, int value);
void gen_pop(Program_State *state, FILE *file);
void gen_push_str(Program_State *state, FILE *file, String_View value);
void gen_indup(Program_State *state, FILE *file, size_t value);
void gen_inswap(Program_State *state, FILE *file, size_t value);
void gen_zjmp(Program_State *state, FILE *file, size_t label);
void gen_jmp(Program_State *state, FILE *file, size_t label);
void gen_while_jmp(Program_State *state, FILE *file, size_t label);
void gen_label(Program_State *state, FILE *file, size_t label);
void gen_func_label(Program_State *state, FILE *file, String_View label);
void gen_func_call(Program_State *state, FILE *file, String_View label);
void gen_while_label(Program_State *state, FILE *file, size_t label);
void strip_off_dot(char *str);
char *append_tasm_ext(char *filename);
Function *get_func(Functions functions, String_View name);
size_t get_func_loc(Functions functions, String_View name);
int get_variable_location(Program_State *state, String_View name);
void gen_expr(Program_State *state, FILE *file, Expr *expr);
void scope_end(Program_State *state, FILE *file);
void gen_program(Program_State *state, Nodes nodes, FILE *file);
void generate(Program_State *state, Program *program, char *filename);

#endif // BACKEND_H