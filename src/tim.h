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
#include <dlfcn.h>


#include "view.h"

#define MAX_STACK_SIZE 1024
#define DATA_START_CAPACITY 16

#define DA_APPEND(da, item) do {                                                       \
    if ((da)->count >= (da)->capacity) {                                               \
        (da)->capacity = (da)->capacity == 0 ? DATA_START_CAPACITY : (da)->capacity*2; \
        (da)->data = custom_realloc((da)->data, (da)->capacity*sizeof(*(da)->data));       \
        ASSERT((da)->data != NULL, "outta ram");                               \
    }                                                                                  \
    (da)->data[(da)->count++] = (item);                                               \
} while (0)

typedef enum {
    INST_NOP = 0,
    INST_PUSH,
    INST_PUSH_STR,
    INST_MOV,
    INST_REF,
    INST_DEREF,
    INST_ALLOC,
    INST_DEALLOC,
    INST_WRITE,
    INST_READ,
    INST_POP,
    INST_DUP,
    INST_INDUP,
    INST_SWAP,
    INST_INSWAP,
    INST_ADD,
    INST_SUB,
    INST_MUL,
    INST_DIV,
    INST_MOD,
	INST_AND,
	INST_OR,
	// TODO GET RID OF _F OPERATIONS AND REPLACE WITH SWITCH OVER DATA_TYPE
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
	INST_LOAD_LIBRARY,
    INST_SS,
    INST_HALT,
    INST_COUNT,
} Inst_Set;

typedef enum {
    INT_TYPE = 0,
    U8_TYPE,	
    U16_TYPE,	
    U32_TYPE,		
    U64_TYPE,			
    FLOAT_TYPE,
	DOUBLE_TYPE,
    CHAR_TYPE,
    PTR_TYPE,
    REGISTER_TYPE,
    TOP_TYPE,
} DataType;
    
typedef union {
    int64_t as_int;
	uint8_t as_u8;
	uint16_t as_u16;
	uint32_t as_u32;
	uint64_t as_u64;
    float as_float;
	double as_double;
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
        if(b.word.type op a.word.type){     \
            push(machine, yes, INT_TYPE);       \
        } else {                                \
            push(machine, no, INT_TYPE);        \
        }                                       \
    } while(0)

#define MATH_OP(as_type, op, data_type) \
    do { \
        b = pop(machine);   \
        a = pop(machine);   \
        Word c = {.as_type = a.word.as_type op b.word.as_type}; \
        push(machine, c, data_type); \
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
	
#define GET_TYPE(var, val) \
		do { \
		switch((var).type) { \
			case INT_TYPE: (val) = (var).word.as_int; break;				\
			case U8_TYPE: (val) = (var).word.as_u8; break;\
			case U16_TYPE: (val) = (var).word.as_u16; break;			\
			case U32_TYPE: (val) = (var).word.as_u32; break;			\
			case U64_TYPE: (val) = (var).word.as_u64; break;			\
			case FLOAT_TYPE: (val) = (var).word.as_float; break;					\
			case DOUBLE_TYPE: (val) = (var).word.as_double; break;											\
			case CHAR_TYPE: (val) = (var).word.as_char; break;												\
			case PTR_TYPE: (val) = (uint64_t)(var).word.as_pointer; break;										\
			default: ASSERT(false, "Unknown type"); \
		} \
	} while(0)

#define TYPE_OP(as_type, return_type, op) \
        do {  \
				switch(a.type) {\
					case CHAR_TYPE:\
					case PTR_TYPE:\
					case U8_TYPE:\
					case U16_TYPE:\
					case U32_TYPE:\
					case U64_TYPE: {\
						uint64_t a_val;\
						uint64_t b_val;		\
						GET_TYPE(a, a_val);			\
						GET_TYPE(b, b_val);\
						push(machine, (Word){.as_type=(a_val op b_val)}, return_type);\
					} break;\
					case INT_TYPE: {\
						int64_t a_val;\
						int64_t b_val;		\
						GET_TYPE(a, a_val);			\
						GET_TYPE(b, b_val);\
						push(machine, (Word){.as_type=(a_val op b_val)}, return_type);\
					} break;\
					case FLOAT_TYPE: {\
						float a_val;\
						float b_val;		\
						GET_TYPE(a, a_val);			\
						GET_TYPE(b, b_val);\
						push(machine, (Word){.as_type=(a_val op b_val)}, return_type);\
					} break;\
					case DOUBLE_TYPE: {\
						double a_val;\
						double b_val;		\
						GET_TYPE(a, a_val);			\
						GET_TYPE(b, b_val);\
						push(machine, (Word){.as_type=(a_val op b_val)}, return_type);\
					} break;\
					default:\
						ASSERT(false, "Unknown type");\
                } \
            } while(0)

#define TIM_ERROR(...) do {				\
	fprintf(stderr, __VA_ARGS__); exit(1);   \
} while (0)


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
} Memory_Cell;

typedef struct Memory {
    struct Memory *next;
    Memory_Cell cell;
} Memory;
	
typedef struct {
	Inst *data;
	size_t count;
	size_t capacity;
} Insts;
	
typedef struct {
	String_View *data;
	size_t count;
	size_t capacity;
} Str_Stack;
	
struct Machine;

typedef void (*native)(struct Machine*);

typedef struct Machine {
    Data stack[MAX_STACK_SIZE];
    int stack_size;
    Str_Stack str_stack;
    size_t return_stack[MAX_STACK_SIZE];
    int return_stack_size;
    size_t program_size;
    
    Memory *memory;

    size_t entrypoint;
    bool has_entrypoint;

    Register registers[AMOUNT_OF_REGISTERS];
		
	native native_ptrs[100];
	size_t native_ptrs_s;

    Insts instructions;
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
void print_stack(Machine *machine);
void write_program_to_file(Machine *machine, char *file_path);
Machine *read_program_from_file(Machine *machine, char *file_path);
void machine_disasm(Machine *machine);
void machine_free(Machine *machine);
void machine_load_native(Machine *machine, native ptr);
void run_instructions(Machine *machine);


#endif // TIM_H

