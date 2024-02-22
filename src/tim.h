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
#define DATA_START_CAPACITY 16

typedef enum {
    INST_NOP = 0,
    INST_PUSH,
    INST_PUSH_PTR,
    INST_PUSH_STR,
    INST_GET_STR,
    INST_MOV,
    INST_MOV_STR,
    INST_REF,
    INST_DEREF,
    INST_ALLOC,
    INST_DEALLOC,
    INST_WRITE,
    INST_READ,
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
    INST_INDEX,
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
    INST_ITOC,
    INST_TOI,
    INST_TOF,
    INST_TOC,
    INST_TOVP,
    INST_CALL,
    INST_RET,
    INST_JMP,
    INST_ZJMP,
    INST_NZJMP,
    INST_PRINT,
    INST_NATIVE,
    INST_ENTRYPOINT,
    INST_HALT,
    INST_COUNT,
} Inst_Set;

typedef enum {
    INT_TYPE = 0,
    FLOAT_TYPE,
    CHAR_TYPE,
    PTR_TYPE,
    REGISTER_TYPE,
    TOP_TYPE,
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
    size_t register_index;
} Inst;

#define CMP_AS_TYPE(type, op) \
    do{         \
        if(a.word.type op b.word.type){     \
            push(machine, yes, INT_TYPE);       \
        } else {                                \
            push(machine, no, INT_TYPE);        \
        }                                       \
    } while(0)

#define MATH_OP(as_type, op, data_type) \
    do{ \
        b = pop(machine);   \
        a = pop(machine);   \
        push(machine, (Word)(a.word.as_type op b.word.as_type), data_type); \
    } while(0)

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
        (da)->data = realloc((da)->data, (da)->capacity*sizeof(*(da)->data));       \
        ASSERT((da)->data != NULL, "outta ram");                               \
    }                                                                                  \
    (da)->data[(da)->count++] = (item);                                               \
} while (0)

#define PRINT_ERROR(message) fprintf(stderr, message); exit(1)


#define AMOUNT_OF_REGISTERS 16 
#define MAX_STRING_SIZE 256

typedef struct {
    Word data;
    DataType data_type;
} Register;
    
typedef struct {
    int8_t *data;
    size_t count;
    size_t capacity;
} Memory;
    
typedef struct {
    size_t len;
    char *data;
} String_View;

typedef struct {
    Data stack[MAX_STACK_SIZE];
    int stack_size;
    String_View str_stack[MAX_STACK_SIZE];
    size_t str_stack_size;
    size_t return_stack[MAX_STACK_SIZE];
    int return_stack_size;
    size_t program_size;
    
    Memory memory;

    size_t entrypoint;
    bool has_entrypoint;

    Register registers[AMOUNT_OF_REGISTERS];

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
