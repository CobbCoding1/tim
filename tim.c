#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAX_STACK_SIZE 1024

typedef enum {
    INST_NOP = 0,
    INST_PUSH,
    INST_POP,
    INST_DUP,
    INST_SWAP,
    INST_ADD,
    INST_SUB,
    INST_MUL,
    INST_DIV,
    INST_MOD,
    INST_CMPE,
    INST_CMPNE,
    INST_CMPG,
    INST_CMPL,
    INST_CMPGE,
    INST_CMPLE,
    INST_JMP,
    INST_ZJMP,
    INST_NZJMP,
    INST_PRINT,
    INST_HALT,
} Inst_Set;

typedef struct {
    Inst_Set type;
    int value;
} Inst;

typedef struct {
    int stack[MAX_STACK_SIZE];
    int stack_size;
    size_t program_size;
    Inst *instructions;
} Machine;

#define DEF_INST_NOP(x) {.type = INST_NOP}
#define DEF_INST_PUSH(x) {.type = INST_PUSH, .value = x}
#define DEF_INST_POP() {.type = INST_POP}
#define DEF_INST_DUP() {.type = INST_DUP}
#define DEF_INST_SWAP() {.type = INST_SWAP}
#define DEF_INST_ADD() {.type = INST_ADD}
#define DEF_INST_SUB() {.type = INST_SUB}
#define DEF_INST_MUL() {.type = INST_MUL}
#define DEF_INST_DIV() {.type = INST_DIV}
#define DEF_INST_MOD() {.type = INST_MOD}
#define DEF_INST_CMPE() {.type = INST_CMPE}
#define DEF_INST_CMPNE() {.type = INST_CMPNE}
#define DEF_INST_CMPG() {.type = INST_CMPG}
#define DEF_INST_CMPL() {.type = INST_CMPL}
#define DEF_INST_CMPGE() {.type = INST_CMPGE}
#define DEF_INST_CMPLE() {.type = INST_CMPLE}
#define DEF_INST_JMP(x) {.type = INST_JMP, .value = x}
#define DEF_INST_ZJMP(x) {.type = INST_ZJMP, .value = x}
#define DEF_INST_NZJMP(x) {.type = INST_NZJMP, .value = x}
#define DEF_INST_PRINT() {.type = INST_PRINT}
#define DEF_INST_HALT(x) {.type = INST_HALT}

Inst program[] = {
    DEF_INST_PUSH(14),
    DEF_INST_PUSH(14),
    DEF_INST_PUSH(14),
    DEF_INST_PUSH(14),
    DEF_INST_PUSH(5),
    DEF_INST_NZJMP(6),
    DEF_INST_PUSH(15),
    DEF_INST_NOP(),
    DEF_INST_NOP(),
    DEF_INST_NOP(),
    DEF_INST_PRINT(),
};
#define PROGRAM_SIZE (sizeof(program)/sizeof(program[0]))


void push(Machine *machine, int value){
    if(machine->stack_size >= MAX_STACK_SIZE){
        fprintf(stderr, "ERROR: Stack Overflow\n");
        exit(1);
    }
    machine->stack[machine->stack_size] = value;
    machine->stack_size++;
}

int pop(Machine *machine){
    if(machine->stack_size <= 0){
        fprintf(stderr, "ERROR: Stack Underflow\n");
        exit(1);
    }
    machine->stack_size--;
    return machine->stack[machine->stack_size];
}

void print_stack(Machine *machine){
    printf("------ STACK\n");
    for(int i = machine->stack_size - 1; i >= 0; i--){
        printf("%d\n", machine->stack[i]);
    }
    printf("------ END OF STACK\n");
}

void write_program_to_file(Machine *machine, char *file_path){
    FILE *file = fopen(file_path, "wb");
    if(file == NULL){
        fprintf(stderr, "ERROR: Could not write to file %s\n", file_path);
        exit(1);
    }

    fwrite(machine->instructions, sizeof(machine->instructions[0]), PROGRAM_SIZE, file);

    fclose(file);
}

Machine *read_program_from_file(Machine *machine, char *file_path){
    FILE *file = fopen(file_path, "rb");
    if(file == NULL){
        fprintf(stderr, "ERROR: Could not read from file %s\n", file_path);
        exit(1);
    }
    Inst *instructions = malloc(sizeof(Inst) * MAX_STACK_SIZE);

    fseek(file, 0, SEEK_END);
    int length = ftell(file);
    fseek(file, 0, SEEK_SET);
    fread(instructions, sizeof(instructions[0]), length / 8, file);

    machine->program_size = length / 8;
    machine->instructions = instructions;

    fclose(file);
    return machine;
}


int main(){
    int a, b;
    Machine *loaded_machine = malloc(sizeof(Machine) * MAX_STACK_SIZE);
    loaded_machine->instructions = program;
    write_program_to_file(loaded_machine, "test.tim");
    loaded_machine = read_program_from_file(loaded_machine, "test.tim");
    for(size_t ip = 0; ip < loaded_machine->program_size; ip++){
        switch(loaded_machine->instructions[ip].type){
            case INST_NOP:
                continue;
                break;
            case INST_PUSH:
                push(loaded_machine, loaded_machine->instructions[ip].value);
                break;
            case INST_POP:
                pop(loaded_machine);
                break;
            case INST_DUP:
                a = pop(loaded_machine);
                push(loaded_machine, a);
                push(loaded_machine, a);
                break;
            case INST_SWAP:
                a = pop(loaded_machine);
                b = pop(loaded_machine);
                push(loaded_machine, a);
                push(loaded_machine, b);
                break;
            case INST_ADD:
                a = pop(loaded_machine);
                b = pop(loaded_machine);
                push(loaded_machine, a + b);
                break;
            case INST_SUB:
                a = pop(loaded_machine);
                b = pop(loaded_machine);
                push(loaded_machine, a - b);
                break;
            case INST_MUL:
                a = pop(loaded_machine);
                b = pop(loaded_machine);
                push(loaded_machine, a * b);
                break;
            case INST_DIV:
                a = pop(loaded_machine);
                b = pop(loaded_machine);
                if(b == 0){
                    fprintf(stderr, "ERROR: Cannot divide by 0\n");
                    exit(1);
                }
                push(loaded_machine, a / b);
                break;
            case INST_MOD:
                a = pop(loaded_machine);
                b = pop(loaded_machine);
                push(loaded_machine, a % b);
                break;
            case INST_CMPE:
                a = pop(loaded_machine);
                b = pop(loaded_machine);
                push(loaded_machine, b);
                push(loaded_machine, a);
                if(a == b){
                    push(loaded_machine, 1);
                } else {
                    push(loaded_machine, 0);
                }
                break;
            case INST_CMPNE:
                a = pop(loaded_machine);
                b = pop(loaded_machine);
                push(loaded_machine, b);
                push(loaded_machine, a);
                if(a != b){
                    push(loaded_machine, 1);
                } else {
                    push(loaded_machine, 0);
                }
                break;
            case INST_CMPG:
                a = pop(loaded_machine);
                b = pop(loaded_machine);
                push(loaded_machine, b);
                push(loaded_machine, a);
                if(a > b){
                    push(loaded_machine, 1);
                } else {
                    push(loaded_machine, 0);
                }
                break;
            case INST_CMPL:
                a = pop(loaded_machine);
                b = pop(loaded_machine);
                push(loaded_machine, b);
                push(loaded_machine, a);
                if(a < b){
                    push(loaded_machine, 1);
                } else {
                    push(loaded_machine, 0);
                }
                break;
            case INST_CMPGE:
                a = pop(loaded_machine);
                b = pop(loaded_machine);
                push(loaded_machine, b);
                push(loaded_machine, a);
                if(a >= b){
                    push(loaded_machine, 1);
                } else {
                    push(loaded_machine, 0);
                }
                break;
            case INST_CMPLE:
                a = pop(loaded_machine);
                b = pop(loaded_machine);
                push(loaded_machine, b);
                push(loaded_machine, a);
                if(a <= b){
                    push(loaded_machine, 1);
                } else {
                    push(loaded_machine, 0);
                }
                break;
            case INST_JMP:
                ip = loaded_machine->instructions[ip].value - 1;
                if(ip + 1 >= loaded_machine->program_size){
                    fprintf(stderr, "ERROR: Cannot jump out of bounds\n");
                    exit(1);
                }
                break;
            case INST_ZJMP:
                if(pop(loaded_machine) == 0){
                    ip = loaded_machine->instructions[ip].value - 1;
                    if(ip + 1 >= loaded_machine->program_size){
                        fprintf(stderr, "ERROR: Cannot jump out of bounds\n");
                        exit(1);
                    }
                } else {
                    continue;
                }
                break;
            case INST_NZJMP:
                if(pop(loaded_machine) != 0){
                    ip = loaded_machine->instructions[ip].value - 1;
                    if(ip + 1 >= loaded_machine->program_size){
                        fprintf(stderr, "ERROR: Cannot jump out of bounds\n");
                        exit(1);
                    }
                } else {
                    continue;
                }
                break;
            case INST_PRINT:
                printf("%d\n", pop(loaded_machine));
                break;
            case INST_HALT:
                ip = loaded_machine->program_size;
                break;
        }
    }
    print_stack(loaded_machine);
    return 0;
}
