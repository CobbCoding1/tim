#ifndef TIM_H
#define TIM_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <inttypes.h>

#define MAX_STACK_SIZE 1024

typedef enum {
    INST_NOP = 0,
    INST_PUSH,
    INST_PUSH_PTR,
    INST_PUSH_STR,
    INST_GET_STR,
    INST_MOV_STR,
    INST_REF,
    INST_DEREF,
    INST_POP,
    INST_POP_STR,
    INST_DUP,
    INST_DUP_STR,
    INST_INDUP,
    INST_INDUP_STR,
    INST_SWAP,
    INST_SWAP_STR,
    INST_INSWAP,
    INST_INSWAP_STR,
    INST_ADD,
    INST_SUB,
    INST_MUL,
    INST_DIV,
    INST_MOD,
    INST_ADD_F,
    INST_SUB_F,
    INST_MUL_F,
    INST_DIV_F,
    INST_MOD_F,
    INST_CMPE,
    INST_CMPNE,
    INST_CMPG,
    INST_CMPL,
    INST_CMPGE,
    INST_CMPLE,
    INST_ITOF,
    INST_FTOI,
    INST_CALL,
    INST_RET,
    INST_JMP,
    INST_ZJMP,
    INST_NZJMP,
    INST_PRINT,
    INST_NATIVE,
    INST_HALT,
    INST_COUNT,
} Inst_Set;

typedef enum {
    INT_TYPE,
    FLOAT_TYPE,
    CHAR_TYPE,
    PTR_TYPE,
} DataType;

typedef union {
    int64_t as_int;
    double as_float;
    char as_char;
    void *as_pointer;
} Word;

typedef struct {
    Word word;
    DataType type; 
} Data;

typedef struct {
    Inst_Set type;
    Word value;
    DataType data_type;
} Inst;

#define CMP_AS_TYPE(type, op) \
    do{         \
        if(a.word.type op b.word.type){     \
            push(machine, yes, INT_TYPE);       \
        } else {                                \
            push(machine, no, INT_TYPE);        \
        }                                       \
    } while(0)

#define MAX_STRING_SIZE 256

typedef struct {
    Data stack[MAX_STACK_SIZE];
    int stack_size;
    char str_stack[MAX_STACK_SIZE][MAX_STRING_SIZE];
    int str_stack_size;
    size_t return_stack[MAX_STACK_SIZE];
    int return_stack_size;
    size_t program_size;
    Inst *instructions;
} Machine;

// helper functions

char *reverse_string(char *str);
int int_to_str(char *str, int *str_index, int64_t x);
void *get_stream(Word stream);

// natives

void native_open(Machine *machine);
void native_write(Machine *machine);
void native_read(Machine *machine);
void native_close(Machine *machine);
void native_malloc(Machine *machine);
void native_free(Machine *machine);
void native_exit(Machine *machine);
void native_itoa(Machine *machine);

// reverse_string

void push_ptr(Machine *machine, Word *value);
void push(Machine *machine, Word value, DataType type);
void push_str(Machine *machine, char *value);
Data pop(Machine *machine);
void index_swap(Machine *machine, int64_t index);
void index_dup(Machine *machine, int64_t index);
char *get_str_from_stack(Machine *machine);
void print_stack(Machine *machine);
void write_program_to_file(Machine *machine, char *file_path);
Machine *read_program_from_file(Machine *machine, char *file_path);
void run_instructions(Machine *machine);


#endif
