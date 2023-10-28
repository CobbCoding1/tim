#include "tim.h"

// native functions

void native_open(Machine *machine){
    int flag_mode = pop(machine).as_int;
    int flag_creation = pop(machine).as_int;
    char *path = (char*)pop(machine).as_pointer;
    int64_t fd = open(path, flag_mode | flag_creation, 0666);
    push(machine, (Word)fd);
}

void native_write(Machine *machine){
    int fd = pop(machine).as_int;
    int length = pop(machine).as_int;
    char *str = get_str_from_stack(machine, length);    
    machine->stack_size -= length;
    write(fd, str, length);
}

void native_read(Machine *machine){
    int fd = pop(machine).as_int;
    int length = pop(machine).as_int;
    char *buffer = pop(machine).as_pointer;
    read(fd, buffer, length);
    push_ptr(machine, (Word*)buffer);
}

void native_close(Machine *machine){
    int64_t fd = pop(machine).as_int;
    close(fd);
}

void native_malloc(Machine *machine){
    int num_of_bytes = pop(machine).as_int;
    void *ptr = malloc(1 * num_of_bytes);
    push(machine, (Word)ptr);
}

void native_free(Machine *machine){
    Word ptr = pop(machine);
    free(ptr.as_pointer);
}

// end native functions

void push_ptr(Machine *machine, Word *value){
    if(machine->stack_size >= MAX_STACK_SIZE){
        fprintf(stderr, "ERROR: Stack Overflow\n");
        exit(1);
    }
    machine->stack[machine->stack_size++].as_pointer = value;
   // machine->stack_size++;
}

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

char *get_str_from_stack(Machine *machine, int length){
    if(length > machine->stack_size){
        fprintf(stderr, "ERROR: Stack Underflow\n");
        exit(1);
    }
    char *buffer = malloc(sizeof(char) * length);
    int buffer_index = 0;
    while(length > 0){
        buffer[buffer_index] = machine->stack[machine->stack_size - length].as_char;
        length--;
        buffer_index++;
    }
    return buffer;
}

void print_stack(Machine *machine){
    printf("------ STACK\n");
    for(int i = machine->stack_size - 1; i >= 0; i--){
        printf("as int: %" PRId64 ", as float: %f, as char: %c, as pointer: %p\n", machine->stack[i].as_int, 
               machine->stack[i].as_float, machine->stack[i].as_char, machine->stack[i].as_pointer);
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

    machine->program_size = length / 16;
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
                b = pop(machine);
                a = pop(machine);
                push(machine, (Word)(fmod(a.as_float, b.as_float)));
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
                printf("as float: %f, as int: %" PRId64 ", as char: %c, as pointer: %p\n", a.as_float, a.as_int, a.as_char, a.as_pointer);
                break;
            case INST_NATIVE:
                switch(machine->instructions[ip].value.as_int){
                    case 0:
                        native_open(machine);
                        break;
                    case 1:
                        native_write(machine);
                        break;
                    case 2:
                        native_read(machine);
                        break;
                    case 3:
                        native_close(machine);
                        break;
                    case 4:
                        native_malloc(machine);
                        break;
                    case 5:
                        native_free(machine);
                        break;
                    default:
                        fprintf(stderr, "error: unexpected native call\n");
                        exit(1);
                }
                break;
            case INST_HALT:
                ip = machine->program_size;
                break;
            case INST_COUNT:
                assert(false);
        }
    }
}
