#include "tim.h"

void push(Machine *machine, Word value){
    if(machine->stack_size >= MAX_STACK_SIZE){
        fprintf(stderr, "ERROR: Stack Overflow\n");
        exit(1);
    }
    machine->stack[machine->stack_size++] = value;
   // machine->stack_size++;
}

Word pop(Machine *machine){
    if(machine->stack_size <= 0){
        fprintf(stderr, "ERROR: Stack Underflow\n");
        exit(1);
    }
    machine->stack_size--;
    return machine->stack[machine->stack_size];
}

void index_swap(Machine *machine, int64_t index){
    if(index > machine->stack_size || index < 0){
        fprintf(stderr, "ERROR: Index out of range\n");
        exit(1);
    }
    Word temp_value = machine->stack[index];
    machine->stack[index] = pop(machine); 
    push(machine, temp_value);
}

void index_dup(Machine *machine, int64_t index){
    if(index > machine->stack_size || index < 0){
        fprintf(stderr, "ERROR: Index out of range\n");
        exit(1);
    }
    push(machine, machine->stack[index]);
}

void print_stack(Machine *machine){
    printf("------ STACK\n");
    for(int i = machine->stack_size - 1; i >= 0; i--){
        printf("as int: %ld, as float: %f, as char: %c\n", machine->stack[i].as_int, machine->stack[i].as_float, machine->stack[i].as_char);
    }
    printf("------ END OF STACK\n");
}

void write_program_to_file(Machine *machine, char *file_path){
    FILE *file = fopen(file_path, "wb");
    if(file == NULL){
        fprintf(stderr, "ERROR: Could not write to file %s\n", file_path);
        exit(1);
    }

    fwrite(machine->instructions, sizeof(machine->instructions[0]), machine->program_size, file);

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
    fread(instructions, sizeof(instructions[0]), length, file);

    machine->program_size = length;
    machine->instructions = instructions;

    fclose(file);
    return machine;
}

void run_instructions(Machine *machine){
    Word a, b;
    Word yes;
    yes.as_int = 1;
    Word no;
    no.as_int = 0;
    for(size_t ip = 0; ip < machine->program_size; ip++){
        switch(machine->instructions[ip].type){
            case INST_NOP:
                continue;
                break;
            case INST_PUSH:
                push(machine, machine->instructions[ip].value);
                break;
            case INST_POP:
                pop(machine);
                break;
            case INST_DUP:
                a = pop(machine);
                push(machine, a);
                push(machine, a);
                break;
            case INST_INDUP:
                index_dup(machine, machine->instructions[ip].value.as_int);
                break;
            case INST_SWAP:
                a = pop(machine);
                b = pop(machine);
                push(machine, a);
                push(machine, b);
                break;
            case INST_INSWAP:
                index_swap(machine, machine->instructions[ip].value.as_int);
                break;
            case INST_ADD:
                b = pop(machine);
                a = pop(machine);
                push(machine, (Word)(a.as_int + b.as_int));
                break;
            case INST_SUB:
                b = pop(machine);
                a = pop(machine);
                push(machine, (Word)(a.as_int - b.as_int));
                break;
            case INST_MUL:
                b = pop(machine);
                a = pop(machine);
                push(machine, (Word)(a.as_int * b.as_int));
                break;
            case INST_DIV:
                b = pop(machine);
                a = pop(machine);
                if(b.as_int == 0){
                    fprintf(stderr, "ERROR: Cannot divide by 0\n");
                    exit(1);
                }
                push(machine, (Word)(a.as_int / b.as_int));
                break;
            case INST_MOD:
                b = pop(machine);
                a = pop(machine);
                push(machine, (Word)(a.as_int % b.as_int));
                break;
            case INST_ADD_F:
                b = pop(machine);
                a = pop(machine);
                push(machine, (Word)(a.as_float + b.as_float));
                break;
            case INST_SUB_F:
                b = pop(machine);
                a = pop(machine);
                push(machine, (Word)(a.as_float - b.as_float));
                break;
            case INST_MUL_F:
                b = pop(machine);
                a = pop(machine);
                push(machine, (Word)(a.as_float * b.as_float));
                break;
            case INST_DIV_F:
                b = pop(machine);
                a = pop(machine);
                push(machine, (Word)(a.as_float / b.as_float));
                break;
            case INST_MOD_F:
                assert(false && "TODO: NOT IMPLEMENTED YET\n");
                //b = pop(machine);
                //a = pop(machine);
                //push(machine, (Word)(modf(a.as_float, b.as_float)));
                break;
            case INST_CMPE:
                a = pop(machine);
                b = pop(machine);
                push(machine, b);
                push(machine, a);
                if(a.as_float == b.as_float){
                    push(machine, yes);
                } else {
                    push(machine, no);
                }
                break;
            case INST_CMPNE:
                a = pop(machine);
                b = pop(machine);
                push(machine, b);
                push(machine, a);
                if(a.as_float != b.as_float){
                    push(machine, yes);
                } else {
                    push(machine, no);
                }
                break;
            case INST_CMPG:
                a = pop(machine);
                b = pop(machine);
                push(machine, b);
                push(machine, a);
                if(a.as_float > b.as_float){
                    push(machine, yes);
                } else {
                    push(machine, no);
                }
                break;
            case INST_CMPL:
                a = pop(machine);
                b = pop(machine);
                push(machine, b);
                push(machine, a);
                if(a.as_float < b.as_float){
                    push(machine, yes);
                } else {
                    push(machine, no);
                }
                break;
            case INST_CMPGE:
                a = pop(machine);
                b = pop(machine);
                push(machine, b);
                push(machine, a);
                if(a.as_float >= b.as_float){
                    push(machine, yes);
                } else {
                    push(machine, no);
                }
                break;
            case INST_CMPLE:
                a = pop(machine);
                b = pop(machine);
                push(machine, b);
                push(machine, a);
                if(a.as_float <= b.as_float){
                    push(machine, yes);
                } else {
                    push(machine, no);
                }
                break;
            case INST_JMP:
                ip = machine->instructions[ip].value.as_int - 1;
                if(ip + 1 >= machine->program_size){
                    fprintf(stderr, "ERROR: Cannot jump out of bounds\n");
                    exit(1);
                }
                break;
            case INST_ZJMP:
                if(pop(machine).as_int == 0){
                    ip = machine->instructions[ip].value.as_int - 1;
                    if(ip + 1 >= machine->program_size){
                        fprintf(stderr, "ERROR: Cannot jump out of bounds\n");
                        exit(1);
                    }
                } else {
                    break;
                }
                break;
            case INST_NZJMP:
                if(pop(machine).as_int != 0){
                    ip = machine->instructions[ip].value.as_int - 1;
                    if(ip + 1 >= machine->program_size){
                        fprintf(stderr, "ERROR: Cannot jump out of bounds\n");
                        exit(1);
                    }
                } else {
                    break;
                }
                break;
            case INST_PRINT:
                a = pop(machine);
                printf("as float: %f, as int: %ld, as char: %c\n", a.as_float, a.as_int, a.as_char);
                break;
            case INST_HALT:
                ip = machine->program_size;
                break;
        }
    }

}

/*
int tim(){
    Machine *loaded_machine = malloc(sizeof(Machine) * MAX_STACK_SIZE);

    loaded_machine->instructions = program;
    write_program_to_file(loaded_machine, "test.tim");
    loaded_machine = read_program_from_file(loaded_machine, "test.tim");
    
    run_instructions(loaded_machine);
    print_stack(loaded_machine);
    return 0;
}
*/
