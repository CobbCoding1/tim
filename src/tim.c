#include "tim.h"

char *str_types[] = {"int", "float", "char", "ptr", "reg", "top"};

void free_cell(Memory *cell) {
    free(cell->cell.data);    
    free(cell);
}

void free_memory(Machine *machine, void *ptr) {
    Memory *cur = machine->memory;
    if(cur == NULL) return;
    while(cur->next != NULL) {
        if(cur->next->cell.data == ptr) {
           Memory *cell = cur->next;
            cur->next = cur->next->next;
            free_cell(cell);
        }
        cur = cur->next;
    }
}
    
void insert_memory(Machine *machine, size_t size) {
    Memory *new = malloc(sizeof(Memory));    
    memset(new, 0, sizeof(Memory));
    new->cell.data = malloc(sizeof(*new->cell.data)*size);
    new->next = machine->memory;
    machine->memory = new;
}

int64_t my_trunc(double num){
    return (int64_t)num;
}

int64_t my_pow(int64_t base, int64_t num2){
    int64_t result = 1;
    for(size_t i = 0; i < (size_t)num2; i++){
        result *= base; 
    }
    return result;
}

double my_fmod(double num1, double num2){
    return num1 - my_trunc(num1 / num2) * num2;
}

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

        fpart = fpart * my_pow(10, afterpoint);
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
    int64_t result = my_pow(power, num);
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
    machine->stack[machine->stack_size].type = PTR_TYPE;
    machine->stack[machine->stack_size++].word.as_pointer = value;
}

Data pop(Machine *machine){
    if(machine->stack_size <= 0){
        PRINT_ERROR("error: stack underflow\n");
    }
    machine->stack_size--;
    return machine->stack[machine->stack_size];
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

void index_dup(Machine *machine, int64_t index){
    if(machine->stack_size <= 0){
        PRINT_ERROR("error: stack underflow\n");
    }
    if(index > machine->stack_size || index < 0){
        PRINT_ERROR("error: index out of range\n");
    }
    push(machine, machine->stack[index].word, machine->stack[index].type);
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
        case FLOAT_TYPE:
            return 1;
        case CHAR_TYPE:
            return 2;
        case PTR_TYPE:
            return 3;
        case REGISTER_TYPE:
        case TOP_TYPE:
        default:
            return -1;        
    }
}

void write_program_to_file(Machine *machine, char *file_path){
    FILE *file = fopen(file_path, "wb");
    if(file == NULL){
        PRINT_ERROR("error: could not write to file\n");
    }
    fwrite(&machine->str_stack_size, sizeof(size_t), 1, file);
    for(int i = 0; i < (int)machine->str_stack_size; i++){
        String_View str = machine->str_stack[i];
        fwrite(&str.len, sizeof(size_t), 1, file);        
        fwrite(str.data, sizeof(char), str.len, file);
    }

    fwrite(&machine->entrypoint, sizeof(size_t), 1, file);
    fwrite(machine->instructions, sizeof(machine->instructions[0]), machine->program_size, file);

    fclose(file);
}

Machine *read_program_from_file(Machine *machine, char *file_path){
    FILE *file = fopen(file_path, "rb");

    if(file == NULL){
        PRINT_ERROR("error: could not read from file\n");
    }

    int index = 0;
    int length;
    fread(&machine->str_stack_size, 1, sizeof(size_t), file);    
    for(size_t i = 0; i < machine->str_stack_size; i++) {
        String_View *str = &machine->str_stack[i];
        fread(&str->len, 1, sizeof(size_t), file);        
        str->data = malloc(sizeof(char)*str->len);
        fread(str->data, sizeof(char), str->len, file);
    }
    index = ftell(file);

    Inst *instructions = malloc(sizeof(Inst) * MAX_STACK_SIZE);

    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, index, SEEK_SET);
    length = length - ftell(file);
    fread(&machine->entrypoint, sizeof(size_t), 1, file);
    length = fread(instructions, sizeof(*instructions), length, file);

    machine->program_size = length;
    machine->instructions = instructions;

    instructions = realloc(instructions, sizeof(Inst) * machine->program_size);

    fclose(file);
    return machine;
}

void run_instructions(Machine *machine){
    Data a, b;
    Word yes;
    yes.as_int = 1;
    Word no;
    no.as_int = 0;
    for(size_t ip = machine->entrypoint; ip < machine->program_size; ip++){
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
            case INST_PUSH_STR: {
                size_t index = machine->instructions[ip].value.as_int;
                String_View str = machine->str_stack[index];
                insert_memory(machine, str.len);
                for(size_t i = 0; i < str.len; i++) {
                    machine->memory->cell.data[i] = str.data[i];
                }
                Word word;
                word.as_pointer = machine->memory->cell.data;
                push(machine, word, PTR_TYPE);
            } break;
            case INST_GET_STR: {
                assert(false && "UNUSED");
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
                assert(false && "UNSUED");
                break;
            case INST_REF: {
                Word *ptr = &machine->stack[machine->stack_size - 1].word;
                push_ptr(machine, ptr);
            } break;
            case INST_DEREF: {
                Word *ptr = machine->stack[machine->stack_size - 1].word.as_pointer;
                Data *ref = (Data*)ptr;
                push(machine, ref->word, ref->type);
                break;
            }
            case INST_ALLOC: {
                a = pop(machine);
                if(a.type != INT_TYPE) {
                    PRINT_ERROR("error: expected int");
                }
                if(a.word.as_int < 0) {
                    PRINT_ERROR("error: size cannot be negative");
                }
                insert_memory(machine, a.word.as_int);
                Word word;
                word.as_pointer = machine->memory->cell.data;
                push(machine, word, PTR_TYPE);
            } break;
            case INST_DEALLOC: {
                Data size = pop(machine);
                if(size.type != PTR_TYPE) {
                    PRINT_ERROR("error: expected int");
                }
                free_memory(machine, size.word.as_pointer);
            } break;
            case INST_WRITE: {
                Data size = pop(machine);                
                Data data = pop(machine);
                if(size.type != INT_TYPE) {
                    PRINT_ERROR("error: expected int");                    
                }
                if(size.word.as_int < 0) {
                    PRINT_ERROR("error: size cannot be negative");                    
                }
                Data ptr_data = pop(machine);
                if(ptr_data.type != PTR_TYPE) {
                    PRINT_ERROR("error: expected ptr");                    
                }
                void *ptr = ptr_data.word.as_pointer;
                memcpy(ptr, &data.word, size.word.as_int);                
            } break;
            case INST_READ: {
                Data size = pop(machine);
                if(size.type != INT_TYPE) {
                    PRINT_ERROR("error: expected int");
                }
                if(size.word.as_int < 0) {
                    PRINT_ERROR("error: size cannot be negative");                    
                }
                Data ptr_data = pop(machine);
                if(ptr_data.type != PTR_TYPE) {
                    PRINT_ERROR("error: expected pointer");
                }
                void *ptr = ptr_data.word.as_pointer;
                Data data = {0};                
                data.type = INT_TYPE;
                memcpy(&data.word, ptr, size.word.as_int);                
                push(machine, data.word, data.type);                
            } break;
            case INST_POP:
                pop(machine);
                break;
            case INST_POP_STR:
                assert(false && "UNUSED");
                break;
            case INST_DUP:
                a = machine->stack[machine->stack_size - 1];
                push(machine, a.word, a.type);
                break;
            case INST_DUP_STR: {
                assert(false && "UNSUED");
                break;
            }
            case INST_INDUP:
                index_dup(machine, machine->stack_size-machine->instructions[ip].value.as_int-1);
                break;
            case INST_INDUP_STR:
                assert(false && "UNUSED");
                break;
            case INST_SWAP: {
                Data temp = machine->stack[machine->stack_size - 1];
                machine->stack[machine->stack_size - 1] = machine->stack[machine->stack_size - 2];
                machine->stack[machine->stack_size - 2] = temp;
            } break;
            case INST_SWAP_STR: {
                assert(false && "UNSUED");
                break;
            }
            case INST_INSWAP:
                index_swap(machine, machine->stack_size-machine->instructions[ip].value.as_int-1);
                break;
            case INST_INSWAP_STR:
                assert(false && "UNUSED");
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
                push(machine, (Word)(my_fmod(a.word.as_float, b.word.as_float)), FLOAT_TYPE);
                break;
            case INST_CMPE: {
                a = machine->stack[machine->stack_size - 1];
                b = machine->stack[machine->stack_size - 2];
                machine->stack_size -= 2;
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
                machine->stack_size -= 2;
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
                machine->stack_size -= 2;
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
                machine->stack_size -= 2;
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
                machine->stack_size -= 2;
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
                machine->stack_size -= 2;
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
                push(machine, a.word, INT_TYPE);
                break;
            case INST_ITOC:
                a = pop(machine);
                a.word.as_char = (char)a.word.as_int;
                push(machine, a.word, CHAR_TYPE);
                break;
            case INST_TOI:
                machine->stack[machine->stack_size-1].type = INT_TYPE;
                break;
            case INST_TOF:
                machine->stack[machine->stack_size-1].type = FLOAT_TYPE;            
                break;
            case INST_TOC:
                machine->stack[machine->stack_size-1].type = CHAR_TYPE;            
                break;
            case INST_TOVP:
                machine->stack[machine->stack_size-1].type = PTR_TYPE;            
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
                printf("as float: %f, as int: %" PRId64 ", as char: %c, as pointer: %p, type: %s\n",
                        a.word.as_float, a.word.as_int, a.word.as_char, a.word.as_pointer, str_types[a.type]);
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
            } break;
            case INST_ENTRYPOINT:
                assert(false);
                break;
            case INST_HALT:
                ip = machine->program_size;
                break;
            case INST_COUNT:
                assert(false);
        }
    }
}
