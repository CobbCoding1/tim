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
        *str_index += 1;
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

int float_to_str(char *str, int *str_index, double x, int afterpoint){
    int ipart = (int64_t)x;
    float fpart = x - (float)ipart;

    int_to_str(str, str_index, ipart);
    if(afterpoint != 0){
        str[*str_index] = '.';
        *str_index += 1;

        fpart = fpart * pow(10, afterpoint);
        if(fpart < 0){
            fpart = -fpart;
        }
        int_to_str(str, str_index, (int64_t)fpart);
    }
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
    Word flag_mode = pop(machine).word;
    if(flag_mode.as_int > MODES_LENGTH - 1){
        fprintf(stderr, "error: mode %" PRId64 "out of bounds\n", flag_mode.as_int);
        exit(1);
    }
    Word path = pop(machine).word;
    char *mode = open_modes[flag_mode.as_int];
    void *fd = fopen(path.as_pointer, mode);
    push(machine, (Word)fd, PTR_TYPE);
}

void native_write(Machine *machine){
    Word stream = pop(machine).word;
    char *str = (char*)pop(machine).word.as_pointer;    
    stream.as_pointer = get_stream(stream);
    int length = strlen(str);
    fwrite(str, 1, length, stream.as_pointer);
}

void native_read(Machine *machine){
    Word ptr = pop(machine).word;
    ptr.as_pointer = get_stream(ptr);
    int length = pop(machine).word.as_int;
    char *buffer = pop(machine).word.as_pointer;
    fread(buffer, 1, length, ptr.as_pointer);
    buffer[length] = '\0';
    push_ptr(machine, (Word*)buffer);
}

void native_close(Machine *machine){
    void *stream = pop(machine).word.as_pointer;
    fclose(stream);
}

void native_malloc(Machine *machine){
    int num_of_bytes = pop(machine).word.as_int;
    void *ptr = malloc(sizeof(char) * num_of_bytes);
    push_ptr(machine, ptr);
}

void native_realloc(Machine *machine){
    void *ptr = pop(machine).word.as_pointer;
    int num_of_bytes = pop(machine).word.as_int;
    void *result = realloc(ptr, sizeof(char) * num_of_bytes);
    push_ptr(machine, result);
}

void native_free(Machine *machine){
    Word ptr = pop(machine).word;
    free(ptr.as_pointer);
}

void native_scanf(Machine *machine){
    char *buffer = pop(machine).word.as_pointer;
    scanf("%s", buffer);
    push_ptr(machine, (Word*)buffer);
}

void native_pow(Machine *machine){
    int64_t power = pop(machine).word.as_int;
    int64_t num = pop(machine).word.as_int;
    int64_t result = pow(power, num);
    push(machine, (Word)result, INT_TYPE);
}

void native_time(Machine *machine){
    time_t seconds;
    seconds = time(NULL);
    push(machine, (Word)(int64_t)seconds, INT_TYPE);
}

void native_exit(Machine *machine){
    int64_t code = pop(machine).word.as_int;
    exit(code);
}

void native_strcmp(Machine *machine){
    char *str1 = (char*)pop(machine).word.as_pointer;
    char *str2 = (char*)pop(machine).word.as_pointer;
    int64_t result = strcmp(str1, str2) == 0;
    push(machine, (Word)result, INT_TYPE);
}

void native_strcpy(Machine *machine){
    char *str = (char*)pop(machine).word.as_pointer;
    void *dest = pop(machine).word.as_pointer;
    char *result = strcpy(dest, str);
    push_ptr(machine, (Word*)result);
}

void native_memcpy(Machine *machine){
    int64_t size = pop(machine).word.as_int;
    void *src = pop(machine).word.as_pointer;
    void *dest = pop(machine).word.as_pointer;
    void *result = memcpy(dest, src, size);
    push_ptr(machine, result);
}

void native_strcat(Machine *machine){
    char *str = (char*)pop(machine).word.as_pointer;
    char *dest = (char*)pop(machine).word.as_pointer;
    char *result = strcat(dest, str);
    push_ptr(machine, (Word*)result);
}

void native_strlen(Machine *machine){
    char *str = (char*)pop(machine).word.as_pointer;
    int64_t result = strlen(str);
    push(machine, (Word)result, INT_TYPE);
}

void native_ftoa(Machine *machine){
    double x = pop(machine).word.as_float;
    int str_index = 0;
    char *str = malloc(sizeof(char) * 64);
    float_to_str(str, &str_index, x, 8);
    str = realloc(str, sizeof(char) * str_index);
    str[str_index] = '\0';
    push_ptr(machine, (Word*)str);
}

void native_itoa(Machine *machine){
    int64_t x = pop(machine).word.as_int;
    int str_index = 0;
    char *str = malloc(sizeof(char) * 64);
    int_to_str(str, &str_index, x);
    str = realloc(str, sizeof(char) * str_index);
    str[str_index] = '\0';
    push_ptr(machine, (Word*)str);
}

void native_assert(Machine *machine){
    int64_t code = pop(machine).word.as_int;
    assert(code);
}

// end native functions

void push(Machine *machine, Word value, DataType type){
    if(machine->stack_size >= MAX_STACK_SIZE){
        PRINT_ERROR("error: stack overflow\n");
    }
    Data data;
    data.word = value;
    data.type = type;
    machine->stack[machine->stack_size++] = data;
}

void push_ptr(Machine *machine, Word *value){
    if(machine->stack_size >= MAX_STACK_SIZE){
        PRINT_ERROR("error: stack overflow\n");
    }
    machine->stack[machine->stack_size++].word.as_pointer = value;
}

void push_str(Machine *machine, char *value){
    if(machine->str_stack_size >= MAX_STACK_SIZE){
        PRINT_ERROR("error: string stack overflow\n");
    }
    strncpy(machine->str_stack[machine->str_stack_size++], value, MAX_STRING_SIZE - 1);
}

Data pop(Machine *machine){
    if(machine->stack_size <= 0){
        PRINT_ERROR("error: stack underflow\n");
    }
    machine->stack_size--;
    return machine->stack[machine->stack_size];
}

char *pop_str(Machine *machine){
    int length = strlen(machine->str_stack[--machine->str_stack_size]);
    char *result = malloc(sizeof(char) * length); 
    if(machine->str_stack_size < 0){
        PRINT_ERROR("error: string stack underflow\n");
    }
    for(int i = 0; i < length; i++){
        result[i] = machine->str_stack[machine->str_stack_size][i];
    }
    result[length] = '\0';
    return result;
}

void index_swap(Machine *machine, int64_t index){
    if(index > machine->stack_size || index < 0){
        PRINT_ERROR("error: index out of range\n");
    }
    Data temp_value = machine->stack[index];
    machine->stack[index] = machine->stack[machine->stack_size - 1]; 
    machine->stack[machine->stack_size - 1] = temp_value;
    //push(machine, temp_value.word, temp_value.type);
}

void index_swap_str(Machine *machine, int64_t index){
    if(index > machine->str_stack_size || index < 0){
        PRINT_ERROR("error: index out of range\n");
    }
    int length = strlen(machine->str_stack[index]);
    char *result = malloc(sizeof(char) * length); 
    for(int i = 0; i < length; i++){
        result[i] = machine->str_stack[index][i];
    }
    result[length] = '\0';
    char *temp_value = pop_str(machine);
    strncpy(machine->str_stack[index], temp_value, MAX_STRING_SIZE - 1);
    push_str(machine, result);
}

void index_dup(Machine *machine, int64_t index){
    if(machine->stack_size <= 0){
        PRINT_ERROR("error: stack underflow\n");
    }
    if(index > machine->stack_size || index < 0){
        PRINT_ERROR("error: index out of range\n");
    }
    push(machine, machine->stack[index].word, machine->stack[index].type);
}

void index_dup_str(Machine *machine, int64_t index){
    if(index > machine->str_stack_size || index < 0){
        PRINT_ERROR("error: index out of range\n");
    }
    push_str(machine, machine->str_stack[index]);
}

char *get_str_from_stack(Machine *machine){
    char *buffer = malloc(sizeof(char) * 16);
    int buffer_index = 0;
    char *current = (char*)&(machine->stack[machine->stack_size].word.as_pointer);
    while(*current != '\0'){
        if(buffer_index > machine->stack_size){
            PRINT_ERROR("error: stack underflow\n");
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
        printf("as int: %" PRId64 ", as float: %f, as char: %c, as pointer: %p\n", machine->stack[i].word.as_int, 
               machine->stack[i].word.as_float, machine->stack[i].word.as_char, machine->stack[i].word.as_pointer);
    }
    printf("------ END OF STACK\n");
}

int cmp_types(Data a){
    switch(a.type){
        case INT_TYPE:
            return 0;
            break;
        case FLOAT_TYPE:
            return 1;
            break;
        case CHAR_TYPE:
            return 2;
            break;
        case PTR_TYPE:
            return 3;
            break;
        case REGISTER_TYPE:
            return -1;
            break;
        case TOP_TYPE:
            return -1;
            break;
    }
    return -1;
}

void write_program_to_file(Machine *machine, char *file_path){
    FILE *file = fopen(file_path, "wb");
    if(file == NULL){
        PRINT_ERROR("error: could not write to file\n");
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
                machine->str_stack[i - 1][char_index] = charRead;
                char_index++;
            }
        }
        machine->str_stack[i - 1][char_index] = '\0';
        if (charRead == EOF || strcmp(machine->str_stack[i - 1], "DATA_END") == 0) {
            index += char_index + i; 
            break;
        }
        index += char_index;
        i++;
    }


    if(file == NULL){
        PRINT_ERROR("error: could not read from file\n");
    }
    Inst *instructions = malloc(sizeof(Inst) * MAX_STACK_SIZE);

    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, index, SEEK_SET);
    length = fread(instructions, sizeof(instructions[0]), length, file);

    machine->program_size = length;
    machine->instructions = instructions;

    instructions = realloc(instructions, sizeof(Inst) * machine->program_size);

    machine->str_stack_size = i - 1;
    fclose(file);
    return machine;
}

void run_instructions(Machine *machine){
    Data a, b;
    Word yes;
    yes.as_int = 1;
    Word no;
    no.as_int = 0;
    for(size_t ip = 0; ip < machine->program_size; ip++){
        //print_stack(machine);
        switch(machine->instructions[ip].type){
            case INST_NOP:
                continue;
                break;
            case INST_PUSH:
                if(machine->instructions[ip].data_type == REGISTER_TYPE){
                    push(machine, machine->registers[machine->instructions[ip].register_index].data, 
                         machine->registers[machine->instructions[ip].register_index].data_type);
                } else {
                    push(machine, machine->instructions[ip].value, machine->instructions[ip].data_type);
                }
                break;
            case INST_PUSH_PTR:
                push_ptr(machine, machine->instructions[ip].value.as_pointer);
                break;
            case INST_PUSH_STR:
                assert(false && "unreachable\n");
                break;
            case INST_GET_STR: {
                int index = machine->instructions[ip].value.as_int;
                if(index >= machine->str_stack_size || index < 0){
                    PRINT_ERROR("error: string stack out of bounds\n");
                    exit(1);
                }
                push_ptr(machine, (void*)machine->str_stack[index]);
                break;
            }
            case INST_MOV:
                if(machine->instructions[ip].data_type == TOP_TYPE){
                    machine->registers[machine->instructions[ip].register_index].data = machine->stack[machine->stack_size - 1].word;
                    machine->registers[machine->instructions[ip].register_index].data_type = machine->stack[machine->stack_size - 1].type;
                } else {
                    machine->registers[machine->instructions[ip].register_index].data = machine->instructions[ip].value;
                    machine->registers[machine->instructions[ip].register_index].data_type = machine->instructions[ip].data_type;
                }
                break;
            case INST_MOV_STR:
                a = pop(machine);
                if(a.type == CHAR_TYPE){
                    char *str = malloc(sizeof(char) * 2);
                    str[0] = a.word.as_char;
                    str[1] = '\0';
                    push_str(machine, str);
                } else {
                    push_str(machine, (char*)a.word.as_pointer);
                }
                break;
            case INST_REF:
                a = machine->stack[machine->stack_size - 1];
                a.type = PTR_TYPE;
                push_ptr(machine, &a.word);
                break;
            case INST_DEREF: {
                Data ptr = machine->stack[machine->stack_size - 1];
                Word *ref = ptr.word.as_pointer;
                push(machine, *ref, PTR_TYPE);
                break;
            }
            case INST_POP:
                pop(machine);
                break;
            case INST_POP_STR:
                pop_str(machine);
                break;
            case INST_DUP:
                a = machine->stack[machine->stack_size - 1];
                push(machine, a.word, a.type);
                break;
            case INST_DUP_STR: {
                char *str1 = pop_str(machine);
                push_str(machine, str1);
                push_str(machine, str1);
                free(str1);
                break;
            }
            case INST_INDUP:
                index_dup(machine, machine->instructions[ip].value.as_int);
                break;
            case INST_INDUP_STR:
                index_dup_str(machine, machine->instructions[ip].value.as_int);
                break;
            case INST_SWAP: {
                Data temp = machine->stack[machine->stack_size - 1];
                machine->stack[machine->stack_size - 1] = machine->stack[machine->stack_size - 2];
                machine->stack[machine->stack_size - 2] = temp;
            } break;
            case INST_SWAP_STR: {
                char *str1 = pop_str(machine);
                char *str2 = pop_str(machine);
                push_str(machine, str1);
                push_str(machine, str2);
                free(str1);
                free(str2);
                break;
            }
            case INST_INSWAP:
                index_swap(machine, machine->instructions[ip].value.as_int);
                break;
            case INST_INSWAP_STR:
                index_swap_str(machine, machine->instructions[ip].value.as_int);
                break;
            case INST_INDEX:
                Data value = pop(machine);
                int64_t index = pop(machine).word.as_int;
                if(index < 0){
                    PRINT_ERROR("error: index cannot be less than 0\n");
                }
                char *arr = (char*)pop(machine).word.as_pointer;
                arr[index] = value.word.as_char;
                push_ptr(machine, (Word*)arr);
                break;
            case INST_ADD:
                MATH_OP(as_int, +, INT_TYPE);
                break;
            case INST_SUB:
                MATH_OP(as_int, -, INT_TYPE);
                break;
            case INST_MUL:
                MATH_OP(as_int, *, INT_TYPE);
                break;
            case INST_DIV:
                if(machine->stack[machine->stack_size - 1].word.as_int == 0){
                    PRINT_ERROR("error: cannot divide by 0\n");
                }
                MATH_OP(as_int, /, INT_TYPE);
                break;
            case INST_MOD:
                if(machine->stack[machine->stack_size - 1].word.as_int == 0){
                    PRINT_ERROR("error: cannot divide by 0\n");
                }
                MATH_OP(as_int, %, INT_TYPE);
                break;
            case INST_ADD_F:
                MATH_OP(as_float, +, FLOAT_TYPE);
                break;
            case INST_SUB_F:
                MATH_OP(as_float, -, FLOAT_TYPE);
                break;
            case INST_MUL_F:
                MATH_OP(as_float, *, FLOAT_TYPE);
                break;
            case INST_DIV_F:
                if(machine->stack[machine->stack_size - 1].word.as_float == 0.0){
                    PRINT_ERROR("error: cannot divide by 0\n");
                }
                MATH_OP(as_float, /, FLOAT_TYPE);
                break;
            case INST_MOD_F:
                if(machine->stack[machine->stack_size - 1].word.as_float == 0.0){
                    PRINT_ERROR("error: cannot divide by 0\n");
                }
                b = pop(machine);
                a = pop(machine);
                push(machine, (Word)(fmod(a.word.as_float, b.word.as_float)), FLOAT_TYPE);
                break;
            case INST_CMPE: {
                a = machine->stack[machine->stack_size - 1];
                b = machine->stack[machine->stack_size - 2];
                int result = (int)a.type;
                switch(result){
                    case 0:
                        CMP_AS_TYPE(as_int, ==);
                        break;
                    case 1:
                        CMP_AS_TYPE(as_float, ==);
                        break;
                    case 2:
                        CMP_AS_TYPE(as_char, ==);
                        break;
                    case 3:
                        CMP_AS_TYPE(as_pointer, ==);
                        break;
                }
                break;
            }
            case INST_CMPNE: {
                a = machine->stack[machine->stack_size - 1];
                b = machine->stack[machine->stack_size - 2];
                int result = (int)a.type;
                switch(result){
                    case 0:
                        CMP_AS_TYPE(as_int, !=);
                        break;
                    case 1:
                        CMP_AS_TYPE(as_float, !=);
                        break;
                    case 2:
                        CMP_AS_TYPE(as_char, !=);
                        break;
                    case 3:
                        CMP_AS_TYPE(as_pointer, !=);
                        break;
                }
                break;
            }
            case INST_CMPG: {
                a = machine->stack[machine->stack_size - 1];
                b = machine->stack[machine->stack_size - 2];
                int result = (int)a.type;
                switch(result){
                    case 0:
                        CMP_AS_TYPE(as_int, >);
                        break;
                    case 1:
                        CMP_AS_TYPE(as_float, >);
                        break;
                    case 2:
                        CMP_AS_TYPE(as_char, >);
                        break;
                    case 3:
                        CMP_AS_TYPE(as_pointer, >);
                        break;
                }
                break;
            }
            case INST_CMPL: {
                a = machine->stack[machine->stack_size - 1];
                b = machine->stack[machine->stack_size - 2];
                int result = (int)a.type;
                switch(result){
                    case 0:
                        CMP_AS_TYPE(as_int, <);
                        break;
                    case 1:
                        CMP_AS_TYPE(as_float, <);
                        break;
                    case 2:
                        CMP_AS_TYPE(as_char, <);
                        break;
                    case 3:
                        CMP_AS_TYPE(as_pointer, <);
                        break;
                }
                break;
            }
            case INST_CMPGE: {
                a = machine->stack[machine->stack_size - 1];
                b = machine->stack[machine->stack_size - 2];
                int result = (int)a.type;
                switch(result){
                    case 0:
                        CMP_AS_TYPE(as_int, >=);
                        break;
                    case 1:
                        CMP_AS_TYPE(as_float, >=);
                        break;
                    case 2:
                        CMP_AS_TYPE(as_char, >=);
                        break;
                    case 3:
                        CMP_AS_TYPE(as_pointer, >=);
                        break;
                }
                break;
            }
            case INST_CMPLE: {
                a = machine->stack[machine->stack_size - 1];
                b = machine->stack[machine->stack_size - 2];
                int result = (int)a.type;
                switch(result){
                    case 0:
                        CMP_AS_TYPE(as_int, <=);
                        break;
                    case 1:
                        CMP_AS_TYPE(as_float, <=);
                        break;
                    case 2:
                        CMP_AS_TYPE(as_char, <=);
                        break;
                    case 3:
                        CMP_AS_TYPE(as_pointer, <=);
                        break;
                }
                break;
            }
            case INST_ITOF:
                a = pop(machine);
                a.word.as_float = (double)a.word.as_int;
                push(machine, a.word, FLOAT_TYPE);
                break;
            case INST_FTOI:
                a = pop(machine);
                a.word.as_int = (int64_t)a.word.as_float;
                printf("%ld\n", a.word.as_int);
                push(machine, a.word, INT_TYPE);
                break;
            case INST_CALL:
                machine->return_stack[machine->return_stack_size++] = ip;
                ip = machine->instructions[ip].value.as_int - 1;
                break;
            case INST_RET:
                ip = machine->return_stack[--machine->return_stack_size];
                break;
            case INST_JMP:
                ip = machine->instructions[ip].value.as_int - 1;
                if(ip + 1 >= machine->program_size){
                    PRINT_ERROR("error: cannot jmp out of bounds\n");
                }
                break;
            case INST_ZJMP:
                if(pop(machine).word.as_int == 0){
                    ip = machine->instructions[ip].value.as_int - 1;
                    if(ip + 1 >= machine->program_size){
                        PRINT_ERROR("error: cannot jmp out of bounds\n");
                    }
                } else {
                    break;
                }
                break;
            case INST_NZJMP:
                if(pop(machine).word.as_int != 0){
                    ip = machine->instructions[ip].value.as_int - 1;
                    if(ip + 1 >= machine->program_size){
                        PRINT_ERROR("error: cannot jmp out of bounds\n");
                    }
                } else {
                    break;
                }
                break;
            case INST_PRINT:
                a = pop(machine);
                printf("as float: %f, as int: %" PRId64 ", as char: %c, as pointer: %p\n", a.word.as_float, a.word.as_int, a.word.as_char, a.word.as_pointer);
                break;
            case INST_NATIVE: {
                void (*native_ptrs[100])(Machine*) = {native_open, native_write, native_read, 
                                                   native_close, native_malloc, native_realloc, 
                                                   native_free, native_scanf, native_pow};
                native_ptrs[10] = native_time;
                native_ptrs[60] = native_exit;
                native_ptrs[90] = native_strcmp;
                native_ptrs[91] = native_strcpy;
                native_ptrs[92] = native_memcpy;
                native_ptrs[93] = native_strcat;
                native_ptrs[94] = native_strlen;
                native_ptrs[98] = native_ftoa;
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
