#include "tim.h"

char *reverse_string(char *str){
    int length = strlen(str);
    int start = 0;
    int end = length - 1;
    char temp;

    while (start < end) {
        temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
    return str;
}

void *get_stream(Word stream){
    switch(stream.as_int){
        case 0:
            return stdin;
            break;
        case 1:
            return stdout;
            break;
        case 2:
            return stderr;
            break;
        default:
            return stream.as_pointer;
            break;
    }
    return stream.as_pointer;
}

// native functions

#define MODES_LENGTH 3
char *open_modes[MODES_LENGTH] = {"r", "w", "wr"};

void native_open(Machine *machine){
    Word flag_mode = pop(machine);
    if(flag_mode.as_int > MODES_LENGTH - 1){
        fprintf(stderr, "error: mode %" PRId64 "out of bounds\n", flag_mode.as_int);
        exit(1);
    }
    Word path = pop(machine);
    char *mode = open_modes[flag_mode.as_int];
    void *fd = fopen(path.as_pointer, mode);
    push(machine, (Word)fd);
}

void native_write(Machine *machine){
    Word stream = pop(machine);
    char *str = get_str_from_stack(machine);    
    stream.as_pointer = get_stream(stream);
    int length = strlen(str);
    machine->stack_size -= length + 1;
    fwrite(str, 1, length, stream.as_pointer);
}

void native_read(Machine *machine){
    Word ptr = pop(machine);
    ptr.as_pointer = get_stream(ptr);
    int length = pop(machine).as_int;
    void *buffer = pop(machine).as_pointer;
    fread(buffer, length, 1, ptr.as_pointer);
    push_ptr(machine, buffer);
}

void native_close(Machine *machine){
    void *stream = pop(machine).as_pointer;
    fclose(stream);
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

void native_exit(Machine *machine){
    int64_t code = pop(machine).as_int;
    exit(code);
}

// end native functions

void push(Machine *machine, Word value){
    if(machine->stack_size >= MAX_STACK_SIZE){
        fprintf(stderr, "ERROR: Stack Overflow\n");
        exit(1);
    }
    machine->stack[machine->stack_size++] = value;
}

void push_ptr(Machine *machine, Word *value){
    if(machine->stack_size >= MAX_STACK_SIZE){
        fprintf(stderr, "ERROR: Stack Overflow\n");
        exit(1);
    }
    machine->stack[machine->stack_size++].as_pointer = value;
}

void push_str(Machine *machine, char *value){
    if(machine->stack_size >= MAX_STACK_SIZE){
        fprintf(stderr, "ERROR: Stack Overflow\n");
        exit(1);
    }
    strncpy(machine->str_stack[machine->str_stack_size++], value, MAX_STRING_SIZE - 1);
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

char *get_str_from_stack(Machine *machine){
    char *buffer = malloc(sizeof(char) * 16);
    int buffer_index = 0;
    char *current = (char*)&(machine->stack[machine->stack_size].as_pointer);
    while(*current != '\0'){
        if(buffer_index > machine->stack_size){
            fprintf(stderr, "ERROR: Stack Underflow\n");
            exit(1);
        }
        buffer[buffer_index] = *current;   
        current = current - sizeof(Word);
        buffer_index++;
    }
    buffer = reverse_string(buffer);
    //buffer = realloc(buffer, sizeof(char) * strlen(buffer));
    buffer_index--;
    buffer[buffer_index] = '\0';
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
    for(int i = 0; i < machine->str_stack_size; i++){
        fwrite(machine->str_stack[i], 1, strlen(machine->str_stack[i]) + 1, file);
    }
    fwrite("DATA_END", 1, strlen("DATA_END") + 1, file);

    fwrite(machine->instructions, sizeof(machine->instructions[0]), machine->program_size, file);

    fclose(file);
}

Machine *read_program_from_file(Machine *machine, char *file_path){
    FILE *file = fopen(file_path, "rb");

    int i = 1;
    int index = 0;
    int length;
    
    while (i < MAX_STACK_SIZE) {
        char charRead;
        int char_index = 0;

        while ((charRead = fgetc(file)) != EOF && charRead != '\0') {
            if (char_index < MAX_STRING_SIZE - 1) { // Avoid buffer overflow
                machine->str_stack[i][char_index] = charRead;
                char_index++;
            }
        }
        machine->str_stack[i][char_index] = '\0';
        if (charRead == EOF || strcmp(machine->str_stack[i], "DATA_END") == 0) {
            index += char_index + i; 
            break;
        }
        index += char_index;
        i++;
    }


    if(file == NULL){
        fprintf(stderr, "ERROR: Could not read from file %s\n", file_path);
        exit(1);
    }
    Inst *instructions = malloc(sizeof(Inst) * MAX_STACK_SIZE);

    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);
    fseek(file, index, SEEK_SET);
    length = fread(instructions, sizeof(instructions[0]), length, file);

    machine->program_size = length;
    machine->instructions = instructions;

    machine->str_stack_size = i;
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
            case INST_PUSH_PTR:
                push_ptr(machine, machine->instructions[ip].value.as_pointer);
                break;
            case INST_PUSH_STR:
                push_str(machine, machine->str_stack[ip]);
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
                    case 60:
                        native_exit(machine);
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
