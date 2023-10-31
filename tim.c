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

int int_to_str(char *str, int *str_index, int64_t x){
    if(x < 0){
        str[*str_index] = '-';
        x = -x;
    }
    if(x > 9){
        int64_t new = x / 10;
        int_to_str(str, str_index, new);
    }
    x = (x % 10) + '0';
    str[*str_index] = (char)(x);
    *str_index += 1;
    return 0;
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
}

// native functions

#define MODES_LENGTH 7
char *open_modes[MODES_LENGTH] = {"r", "w", "wr", "a", "rb", "wb", "ab"};

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
    char *str = (char*)pop(machine).as_pointer;    
    stream.as_pointer = get_stream(stream);
    int length = strlen(str);
    fwrite(str, 1, length, stream.as_pointer);
}

void native_read(Machine *machine){
    Word ptr = pop(machine);
    ptr.as_pointer = get_stream(ptr);
    int length = pop(machine).as_int;
    char *buffer = pop(machine).as_pointer;
    fread(buffer, 1, length, ptr.as_pointer);
    buffer[length] = '\0';
    push_ptr(machine, (Word*)buffer);
}

void native_close(Machine *machine){
    void *stream = pop(machine).as_pointer;
    fclose(stream);
}

void native_malloc(Machine *machine){
    int num_of_bytes = pop(machine).as_int;
    void *ptr = malloc(sizeof(char) * num_of_bytes);
    push_ptr(machine, ptr);
}

void native_realloc(Machine *machine){
    void *ptr = pop(machine).as_pointer;
    int num_of_bytes = pop(machine).as_int;
    void *result = realloc(ptr, sizeof(char) * num_of_bytes);
    push_ptr(machine, result);
}

void native_free(Machine *machine){
    Word ptr = pop(machine);
    free(ptr.as_pointer);
}

void native_time(Machine *machine){
    time_t seconds;
    seconds = time(NULL);
    push(machine, (Word)seconds);
}

void native_exit(Machine *machine){
    int64_t code = pop(machine).as_int;
    exit(code);
}

void native_strcmp(Machine *machine){
    char *str1 = (char*)pop(machine).as_pointer;
    char *str2 = (char*)pop(machine).as_pointer;
    int64_t result = strcmp(str1, str2) == 0;
    push(machine, (Word)result);
}

void native_strcpy(Machine *machine){
    char *str = (char*)pop(machine).as_pointer;
    void *dest = pop(machine).as_pointer;
    char *result = strcpy(dest, str);
    push_ptr(machine, (Word*)result);
}

void native_memcpy(Machine *machine){
    int64_t size = pop(machine).as_int;
    void *src = pop(machine).as_pointer;
    void *dest = pop(machine).as_pointer;
    void *result = memcpy(dest, src, size);
    push_ptr(machine, result);
}

void native_strcat(Machine *machine){
    char *str = (char*)pop(machine).as_pointer;
    char *dest = (char*)pop(machine).as_pointer;
    char *result = strcat(dest, str);
    push_ptr(machine, (Word*)result);
}

void native_strlen(Machine *machine){
    char *str = (char*)pop(machine).as_pointer;
    int64_t result = strlen(str);
    push(machine, (Word)result);
}

void native_itoa(Machine *machine){
    int64_t x = pop(machine).as_int;
    int str_index = 0;
    char *str = malloc(sizeof(char) * 64);
    int_to_str(str, &str_index, x);
    str = realloc(str, sizeof(char) * str_index);
    str[str_index] = '\0';
    push_ptr(machine, (Word*)str);
}

void native_assert(Machine *machine){
    int64_t code = pop(machine).as_int;
    printf("%ld\n", code);
    assert(code);
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
    buffer_index--;
    buffer = realloc(buffer, sizeof(char) * buffer_index);
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
    fseek(file, index, SEEK_SET);
    length = fread(instructions, sizeof(instructions[0]), length, file);

    machine->program_size = length;
    machine->instructions = instructions;

    instructions = realloc(instructions, sizeof(Inst) * machine->program_size);

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
                assert(false && "unreachable\n");
                break;
            case INST_GET_STR: {
                int index = machine->instructions[ip].value.as_int + 1;
                push_ptr(machine, (void*)machine->str_stack[index]);
                break;
            }
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
            case INST_NATIVE: {
                void (*native_ptrs[100])(Machine*) = {native_open, native_write, native_read, 
                                                   native_close, native_malloc, native_realloc, 
                                                   native_free};
                native_ptrs[10] = native_time;
                native_ptrs[60] = native_exit;
                native_ptrs[90] = native_strcmp;
                native_ptrs[91] = native_strcpy;
                native_ptrs[92] = native_memcpy;
                native_ptrs[93] = native_strcat;
                native_ptrs[94] = native_strlen;
                native_ptrs[99] = native_itoa;
                native_ptrs[100] = native_assert;
                (*native_ptrs[machine->instructions[ip].value.as_int])(machine);
                break;
            }
            case INST_HALT:
                ip = machine->program_size;
                break;
            case INST_COUNT:
                assert(false);
        }
    }
}
