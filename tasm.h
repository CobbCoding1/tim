#ifndef TASM_H
#define TASM_H

#include <stdio.h>
#include <stdlib.h>

#include "tasmlexer.h"
#include "tasmparser.h"
#include "tim.h"

void push_program(Inst program[], int *program_size, Inst value);
Inst *generate_instructions(ParseList *head, int *program_size, Data str_stack[MAX_STACK_SIZE]);
char *chop_file_by_dot(char *file_name);
int main(int argc, char *argv[]);

#endif
