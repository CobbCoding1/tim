#include "tim.h"
#define VIEW_IMPLEMENTATION
#include "view.h"

char *str_types[] = {"int", "float", "char", "ptr", "reg", "top"};

char *instructions[INST_COUNT] = {
    "nop",
    "push",
    "push_str",
    "mov",
    "ref",
    "deref",
    "alloc",
    "dealloc",
    "write",
    "read",
    "pop",
    "dup",
    "indup",
    "swap",
    "inswap",
    "add",
    "sub",
    "mul",
    "div",
    "mod",
	"and",
	"or",
    "add_f",
    "sub_f",
    "mul_f",
    "div_f",
    "mod_f",
    "cmpe",
    "cmpne",
    "cmpg",
    "cmpl",
    "cmpge",
    "cmple",
    "itof",
    "ftoi",
    "itoc",
    "toi",
    "tof",
    "toc",
    "tovp",
    "call",
    "ret",
    "jmp",
    "zjmp",
    "nzjmp",
    "print",
    "native",
    "entrypoint",
	"load_lib",
    "ss",
    "halt",
};

bool has_operand[INST_COUNT] = {
     false,       //    "nop",
     true,        //    "push",
     true,        //    "push_str",
     true,        //    "mov",
     false,       //    "ref",
     false,       //    "deref",
     false,       //    "alloc",
     false,       //    "dealloc",
     false,       //    "write",
     false,       //    "read",
     false,       //    "pop",
     false,       //    "dup",
     false,       //    "indup",
     false,       //    "swap",
     false,       //    "inswap",
     false,       //    "add",
     false,       //    "sub",
     false,       //    "mul",
     false,       //    "div",
     false,       //    "mod",
	 false,       //    "and",
	 false,	   //    "or",
     false,       //    "add_f",
     false,       //    "sub_f",
     false,       //    "mul_f",
     false,       //    "div_f",
     false,       //    "mod_f",
     false,       //    "cmpe",
     false,       //    "cmpne",
     false,       //    "cmpg",
     false,       //    "cmpl",
     false,       //    "cmpge",
     false,       //    "cmple",
     false,       //    "itof",
     false,       //    "ftoi",
     false,       //    "itoc",
     false,       //    "toi",
     false,       //    "tof",
     false,       //    "toc",
     false,       //    "tovp",
     true,        //    "call",
     false,       //    "ret",
     true,        //    "jmp",
     true,        //    "zjmp",
     true,        //    "nzjmp",
     false,       //    "print",
     true,        //    "native",
     true,        //    "entrypoint",
	 false,	   //    "load_lib",
     false,       //    "ss",
     false,       //    "halt",
};

void free_cell(Memory **cell) {
    free((*cell)->cell.data);    
    free(*cell);
}

void free_memory(Machine *machine, void *ptr) {
    Memory *cur = machine->memory;
    if(cur == NULL) goto defer;
    if(cur->cell.data == ptr) {
        machine->memory = cur->next;
        free_cell(&cur);
        return;
    }
    while(cur->next != NULL) {
        if(cur->next->cell.data == ptr) {
            Memory *cell = cur->next;
            cur->next = cur->next->next;
            free_cell(&cell);
            return;
        }
        cur = cur->next;
    }
defer:
    TIM_ERROR("could not free pointer\n");
}
    
void insert_memory(Machine *machine, size_t size) {
    Memory *new = malloc(sizeof(Memory));    
    memset(new, 0, sizeof(Memory));
    new->cell.data = malloc(sizeof(*new->cell.data)*size);
    memset(new->cell.data, 0, sizeof(*new->cell.data)*size);
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
        fprintf(stderr, "error: mode %ld out of bounds\n", flag_mode.as_int);
        exit(1);
    }
    Word path = pop(machine).word;
    char *mode = open_modes[flag_mode.as_int];
    void *fd = fopen(path.as_pointer, mode);
    Word w = {.as_pointer=fd};
    push(machine, w, PTR_TYPE);
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
    Word w = {.as_int=result};
    push(machine, w, INT_TYPE);
}

void native_time(Machine *machine){
    time_t seconds;
    seconds = time(NULL);
    Word w = {.as_int=seconds};
    push(machine, w, INT_TYPE);
}

void native_exit(Machine *machine){
    int64_t code = pop(machine).word.as_int;
    exit(code);
}

// end native functions

void push(Machine *machine, Word value, DataType type){
    if(machine->stack_size >= MAX_STACK_SIZE){
        TIM_ERROR("error: stack overflow\n");
    }
    Data data;
    data.word = value;
    data.type = type;
    machine->stack[machine->stack_size++] = data;
}

void push_ptr(Machine *machine, Word *value){
    if(machine->stack_size >= MAX_STACK_SIZE){
        TIM_ERROR("error: stack overflow\n");
    }
    machine->stack[machine->stack_size].type = PTR_TYPE;
    machine->stack[machine->stack_size++].word.as_pointer = value;
}

Data pop(Machine *machine){
    if(machine->stack_size <= 0){
        TIM_ERROR("error: stack underflow\n");
    }
    machine->stack_size--;
    return machine->stack[machine->stack_size];
}

void index_swap(Machine *machine, int64_t index){
    if(index > machine->stack_size || index < 0){
        TIM_ERROR("error: index out of range\n");
    }
    Data temp_value = machine->stack[index];
    machine->stack[index] = machine->stack[machine->stack_size - 1]; 
    machine->stack[machine->stack_size - 1] = temp_value;
}

void index_dup(Machine *machine, int64_t index){
    if(machine->stack_size <= 0){
        TIM_ERROR("error: stack underflow\n");
    }
    if(index > machine->stack_size || index < 0){
        TIM_ERROR("error: index out of range\n");
    }
    push(machine, machine->stack[index].word, machine->stack[index].type);
}

void print_stack(Machine *machine){
    printf("------ STACK\n");
    for(int i = machine->stack_size - 1; i >= 0; i--){
        printf("as int: %ld, as float: %f, as char: %c, as pointer: %p\n", machine->stack[i].word.as_int, 
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
        TIM_ERROR("error: could not write to file\n");
    }
    fwrite(&machine->str_stack.count, sizeof(size_t), 1, file);
    for(int i = 0; i < (int)machine->str_stack.count; i++){
        String_View str = machine->str_stack.data[i];
        fwrite(&str.len, sizeof(size_t), 1, file);        
        fwrite(str.data, sizeof(char), str.len, file);
    }

    fwrite(&machine->entrypoint, sizeof(size_t), 1, file);
    fwrite(machine->instructions.data, sizeof(machine->instructions.data[0]), machine->program_size, file);

    fclose(file);
}

Machine *read_program_from_file(Machine *machine, char *file_path){
    FILE *file = fopen(file_path, "rb");

    if(file == NULL){
        TIM_ERROR("error: could not read from file\n");
    }

    int index = 0;
    size_t length;
    fread(&machine->str_stack.count, 1, sizeof(size_t), file);    
	machine->str_stack.data = malloc(sizeof(String_View)*machine->str_stack.count);
    for(size_t i = 0; i < machine->str_stack.count; i++) {
        size_t len = 0;
        fread(&len, 1, sizeof(size_t), file);        
        char *str = malloc(sizeof(char)*len);
        fread(str, sizeof(char), len, file);
        machine->str_stack.data[i] = view_create(str, len);
    }
    index = ftell(file);


    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, index, SEEK_SET);
    length = length - ftell(file);
    fread(&machine->entrypoint, sizeof(size_t), 1, file);
    Inst *instructions = malloc(sizeof(Inst) * length);		
    length = fread(instructions, sizeof(*instructions), length, file);

    machine->program_size = length;	
    machine->instructions.data = instructions;

	fclose(file);
    return machine;
}

void handle_char_print(char c) {
	switch(c) {
		case '\n':
			putc('\\', stdout);	
			putc('n', stdout);
			break;
		case '\t':
			putc('\\', stdout);	
			putc('t', stdout);
			break;
		case '\v':
			putc('\v', stdout);	
			putc('n', stdout);
			break;
		case '\b':
			putc('\b', stdout);	
			putc('n', stdout);
			break;
		case '\r':
			putc('\\', stdout);	
			putc('r', stdout);
			break;
		case '\f':
			putc('\\', stdout);	
			putc('f', stdout);
			break;
		case '\a':
			putc('\\', stdout);	
			putc('a', stdout);
			break;
		case '\\':
			putc('\\', stdout);	
			putc('\\', stdout);
			break;
		case '\?':
			putc('\\', stdout);	
			putc('?', stdout);
			break;
		case '\'':
			putc('\\', stdout);	
			putc('\'', stdout);
			break;
		case '\"':
			putc('\\', stdout);	
			putc('"', stdout);
			break;
		case '\0':
			putc('\\', stdout);	
			putc('0', stdout);
			break;
		default:
			putc(c, stdout);
	}
}
	
void machine_disasm(Machine *machine) {
	for(size_t i = machine->entrypoint; i < machine->program_size; i++) {
		printf("%s", instructions[machine->instructions.data[i].type]);
		if(has_operand[machine->instructions.data[i].type]) {
			putc(' ', stdout);
			switch(machine->instructions.data[i].data_type) {
				case U64_TYPE:
				case U32_TYPE:		
				case U16_TYPE:		
				case U8_TYPE:				
				case INT_TYPE: {
					int64_t value = machine->instructions.data[i].value.as_int;				
					if(machine->instructions.data[i].type == INST_PUSH_STR) {
						String_View string = machine->str_stack.data[value];
						putc('"', stdout);
						for(size_t j = 0; j < string.len-1; j++) {
							handle_char_print(string.data[j]);
						}
						putc('"', stdout);
						break;
					}
					printf("%ld", value);				
				} break;
				case DOUBLE_TYPE:
				case FLOAT_TYPE: {
					printf("%f", machine->instructions.data[i].value.as_double);				
				} break;
				case CHAR_TYPE: {
					putc('\'', stdout);
					handle_char_print(machine->instructions.data[i].value.as_char);
					putc('\'', stdout);
				} break;				
				case PTR_TYPE: {
					printf("%p", machine->instructions.data[i].value.as_pointer);				
				} break;
				default:
					assert(false && "UNReaCHABLE");
			}
		}
		printf("\n");
	}
}

void machine_free(Machine *machine) {
	Memory *cur = machine->memory;
	while(cur != NULL) {
		Memory *old = cur;
		cur = cur->next;
		free_cell(&old);
	}
	free(machine->instructions.data);
	free(machine->str_stack.data);
} 

void machine_load_native(Machine *machine, native ptr) {
	ASSERT(ptr != NULL, "function pointer cannot be null: %s", dlerror());
	machine->native_ptrs[machine->native_ptrs_s++] = ptr;	
}

void run_instructions(Machine *machine) {
	machine_load_native(machine, native_write);
	machine_load_native(machine, native_exit);
    Data a, b;
    for(size_t ip = machine->entrypoint; ip < machine->program_size; ip++){
        //print_stack(machine);
        switch(machine->instructions.data[ip].type){
            case INST_NOP:
                continue;
                break;
            case INST_PUSH:
                if(machine->instructions.data[ip].data_type == REGISTER_TYPE){
                    push(machine, machine->registers[machine->instructions.data[ip].register_index].data, 
                         machine->registers[machine->instructions.data[ip].register_index].data_type);
                } else {
                    push(machine, machine->instructions.data[ip].value, machine->instructions.data[ip].data_type);
                }
                break;
            case INST_PUSH_STR: {
                size_t index = machine->instructions.data[ip].value.as_int;
                String_View str = machine->str_stack.data[index];
                insert_memory(machine, str.len+1);
                for(size_t i = 0; i < str.len; i++) {
                    machine->memory->cell.data[i] = str.data[i];
                }
				machine->memory->cell.data[str.len] = '\0';
                Word word;
                word.as_pointer = machine->memory->cell.data;
                push(machine, word, PTR_TYPE);
            } break;
            case INST_MOV:
                if(machine->instructions.data[ip].data_type == TOP_TYPE){
                    machine->registers[machine->instructions.data[ip].register_index].data = machine->stack[machine->stack_size - 1].word;
                    machine->registers[machine->instructions.data[ip].register_index].data_type = machine->stack[machine->stack_size - 1].type;
                } else {
                    machine->registers[machine->instructions.data[ip].register_index].data = machine->instructions.data[ip].value;
                    machine->registers[machine->instructions.data[ip].register_index].data_type = machine->instructions.data[ip].data_type;
                }
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
                    TIM_ERROR("error: expected int");
                }
				uint64_t val = a.word.as_int;
                insert_memory(machine, val);
                Word word;
                word.as_pointer = machine->memory->cell.data;
                push(machine, word, PTR_TYPE);
            } break;
            case INST_DEALLOC: {
                Data ptr = pop(machine);
                if(ptr.type != PTR_TYPE) {
                    TIM_ERROR("error: expected ptr");
                }
                free_memory(machine, ptr.word.as_pointer);
            } break;
            case INST_WRITE: {
                Data size = pop(machine);                
                Data data = pop(machine);
                if(size.type != INT_TYPE) {
                    TIM_ERROR("error: expected int");                    
                }
                if(size.word.as_int < 0) {
                    TIM_ERROR("error: size cannot be negative");                    
                }
                Data ptr_data = pop(machine);
                if(ptr_data.type != PTR_TYPE) {
                    TIM_ERROR("error: expected ptr");                    
                }
				uint64_t index = size.word.as_int;
                void *ptr = ptr_data.word.as_pointer;
                memcpy(ptr, &data.word, index);                
            } break;
            case INST_READ: {
                Data size = pop(machine);
                if(size.type != INT_TYPE) {
                    TIM_ERROR("error: expected int");
                }
                if(size.word.as_int < 0) {
                    TIM_ERROR("error: size cannot be negative");                    
                }
                Data ptr_data = pop(machine);
                if(ptr_data.type != PTR_TYPE) {
                    TIM_ERROR("error: expected pointer");
                }
				uint64_t index = size.word.as_int;		
                void *ptr = ptr_data.word.as_pointer;
                Data data = {0};                
                data.type = INT_TYPE;
                memcpy(&data.word, ptr, index);                
                push(machine, data.word, data.type);                
            } break;
            case INST_POP:
                pop(machine);
                break;
            case INST_DUP:
                a = machine->stack[machine->stack_size - 1];
                push(machine, a.word, a.type);
                break;
            case INST_INDUP: {
				Data index = pop(machine);
				if(index.type != INT_TYPE) {
					TIM_ERROR("error: expected int");
				}
                index_dup(machine, machine->stack_size-index.word.as_int-1);
            } break;
            case INST_SWAP: {
                Data temp = machine->stack[machine->stack_size - 1];
                machine->stack[machine->stack_size - 1] = machine->stack[machine->stack_size - 2];
                machine->stack[machine->stack_size - 2] = temp;
            } break;
            case INST_INSWAP: {
				Data index = pop(machine);
				if(index.type != INT_TYPE) {
					TIM_ERROR("error: expected int");
				}
                index_swap(machine, machine->stack_size-index.word.as_int-1);
            } break;
            case INST_ADD:
				if(machine->stack_size < 1) TIM_ERROR("error: stack underflow\n");
                a = machine->stack[machine->stack_size - 1];
                b = machine->stack[machine->stack_size - 2];
                machine->stack_size -= 2;
				switch(a.type) {
					case PTR_TYPE:
                    case U64_TYPE:
                        TYPE_OP(as_u64, U64_TYPE, +);
                        break;
					case CHAR_TYPE:
                    case U8_TYPE:
                        TYPE_OP(as_u8, U8_TYPE, +);
                        break;
                    case U16_TYPE:
                        TYPE_OP(as_u16, U16_TYPE, +);
                        break;
                    case U32_TYPE:
                        TYPE_OP(as_u32, U32_TYPE, +);
                        break;
					case INT_TYPE:
                        TYPE_OP(as_int, INT_TYPE, +);
						break;
					case FLOAT_TYPE:
                        TYPE_OP(as_float, FLOAT_TYPE, +);
						break;
					case DOUBLE_TYPE:
                        TYPE_OP(as_double, DOUBLE_TYPE, +);
						break;
					default:
						TIM_ERROR("error: not right...\n");
				}
                break;
            case INST_SUB:
				if(machine->stack_size < 1) TIM_ERROR("error: stack underflow\n");
                b = machine->stack[machine->stack_size - 1];
                a = machine->stack[machine->stack_size - 2];
                machine->stack_size -= 2;
				switch(a.type) {
					case PTR_TYPE:
                    case U64_TYPE:
                        TYPE_OP(as_u64, U64_TYPE, -);
                        break;
					case CHAR_TYPE:
                    case U8_TYPE:
                        TYPE_OP(as_u8, U8_TYPE, -);
                        break;
                    case U16_TYPE:
                        TYPE_OP(as_u16, U16_TYPE, -);
                        break;
                    case U32_TYPE:
                        TYPE_OP(as_u32, U32_TYPE, -);
                        break;
					case INT_TYPE:
                        TYPE_OP(as_int, INT_TYPE, -);
						break;
					case FLOAT_TYPE:
                        TYPE_OP(as_float, FLOAT_TYPE, -);
						break;
					case DOUBLE_TYPE:
                        TYPE_OP(as_double, DOUBLE_TYPE, -);
						break;
					default:
						TIM_ERROR("error: not right...\n");
				}
                break;
            case INST_MUL:
				if(machine->stack_size < 1) TIM_ERROR("error: stack underflow\n");
                b = machine->stack[machine->stack_size - 1];
                a = machine->stack[machine->stack_size - 2];
                machine->stack_size -= 2;
				switch(a.type) {
					case PTR_TYPE:
                    case U64_TYPE:
                        TYPE_OP(as_u64, U64_TYPE, *);
                        break;
					case CHAR_TYPE:
                    case U8_TYPE:
                        TYPE_OP(as_u8, U8_TYPE, *);
                        break;
                    case U16_TYPE:
                        TYPE_OP(as_u16, U16_TYPE, *);
                        break;
                    case U32_TYPE:
                        TYPE_OP(as_u32, U32_TYPE, *);
                        break;
					case INT_TYPE:
                        TYPE_OP(as_int, INT_TYPE, *);
						break;
					case FLOAT_TYPE:
                        TYPE_OP(as_float, FLOAT_TYPE, *);
						break;
					case DOUBLE_TYPE:
                        TYPE_OP(as_double, DOUBLE_TYPE, *);
						break;
					default:
						TIM_ERROR("error: not right...\n");
				}
                break;
            case INST_DIV:
				if(machine->stack_size < 1) TIM_ERROR("error: stack underflow\n");
                if(machine->stack[machine->stack_size - 1].word.as_int == 0) TIM_ERROR("error: cannot divide by 0\n");
                b = machine->stack[machine->stack_size - 1];
                a = machine->stack[machine->stack_size - 2];
                machine->stack_size -= 2;
				switch(a.type) {
					case PTR_TYPE:
                    case U64_TYPE:
                        TYPE_OP(as_u64, U64_TYPE, /);
                        break;
					case CHAR_TYPE:
                    case U8_TYPE:
                        TYPE_OP(as_u8, U8_TYPE, /);
                        break;
                    case U16_TYPE:
                        TYPE_OP(as_u16, U16_TYPE, /);
                        break;
                    case U32_TYPE:
                        TYPE_OP(as_u32, U32_TYPE, /);
                        break;
					case INT_TYPE:
                        TYPE_OP(as_int, INT_TYPE, /);
						break;
					case FLOAT_TYPE:
                        TYPE_OP(as_float, FLOAT_TYPE, /);
						break;
					case DOUBLE_TYPE:
                        TYPE_OP(as_double, DOUBLE_TYPE, /);
						break;
					default:
						TIM_ERROR("error: not right...\n");
				}
                break;
            case INST_MOD:
				if(machine->stack_size < 1) TIM_ERROR("error: stack underflow\n");
                if(machine->stack[machine->stack_size - 1].word.as_int == 0) TIM_ERROR("error: cannot divide by 0\n");
                MATH_OP(as_int, %, INT_TYPE);
                break;
            case INST_AND:
				if(machine->stack_size < 1) TIM_ERROR("error: stack underflow\n");
                MATH_OP(as_int, &&, INT_TYPE);
                break;
            case INST_OR:
				if(machine->stack_size < 1) TIM_ERROR("error: stack underflow\n");
                MATH_OP(as_int, ||, INT_TYPE);
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
                    TIM_ERROR("error: cannot divide by 0\n");
                }
                MATH_OP(as_float, /, FLOAT_TYPE);
                break;
            case INST_MOD_F:
                if(machine->stack[machine->stack_size - 1].word.as_float == 0.0){
                    TIM_ERROR("error: cannot divide by 0\n");
                }
                b = pop(machine);
                a = pop(machine);
                Word c = {.as_float=my_fmod(a.word.as_float, b.word.as_float)};
                push(machine, c, FLOAT_TYPE);
                break;
            case INST_CMPE: {
                b = machine->stack[machine->stack_size - 1];
                a = machine->stack[machine->stack_size - 2];
                machine->stack_size -= 2;
				TYPE_OP(as_u8, U8_TYPE, ==);
            } break;
            case INST_CMPNE: {
                b = machine->stack[machine->stack_size - 1];
                a = machine->stack[machine->stack_size - 2];
                machine->stack_size -= 2;
				TYPE_OP(as_u8, U8_TYPE, !=);
            } break;
            case INST_CMPG: {
                b = machine->stack[machine->stack_size - 1];
                a = machine->stack[machine->stack_size - 2];
                machine->stack_size -= 2;
				TYPE_OP(as_u8, U8_TYPE, >);
            } break;
            case INST_CMPL: {
                b = machine->stack[machine->stack_size - 1];
                a = machine->stack[machine->stack_size - 2];
                machine->stack_size -= 2;
				TYPE_OP(as_u8, U8_TYPE, <);
            } break;
            case INST_CMPGE: {
                b = machine->stack[machine->stack_size - 1];
                a = machine->stack[machine->stack_size - 2];
                machine->stack_size -= 2;
				TYPE_OP(as_u8, U8_TYPE, >=);
            } break;
            case INST_CMPLE: {
                b = machine->stack[machine->stack_size - 1];
                a = machine->stack[machine->stack_size - 2];
                machine->stack_size -= 2;
				TYPE_OP(as_u8, U8_TYPE, <=);
            } break;
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
                ip = machine->instructions.data[ip].value.as_int - 1;
                break;
            case INST_RET:
                ip = machine->return_stack[--machine->return_stack_size];
                break;
            case INST_JMP:
				if(machine->instructions.data[ip].value.as_int == 0) TIM_ERROR("error: cannot jump to 0\n");
                ip = machine->instructions.data[ip].value.as_int - 1;
                if(ip + 1 >= machine->program_size){
                    TIM_ERROR("error: cannot jmp out of bounds to: %ld\n", machine->instructions.data[ip].value.as_int);
                }
                break;
            case INST_ZJMP:
				if(machine->instructions.data[ip].value.as_int == 0) TIM_ERROR("error: cannot jump to 0\n");					
                if(pop(machine).word.as_int == 0){
                    ip = machine->instructions.data[ip].value.as_int - 1;
                    if(ip + 1 >= machine->program_size){
	                    TIM_ERROR("error: cannot zjmp out of bounds to: %ld\n", machine->instructions.data[ip].value.as_int);							
                    }
                } else {
                    break;
                }
                break;
            case INST_NZJMP:
				if(machine->instructions.data[ip].value.as_int == 0) TIM_ERROR("error: cannot jump to 0\n");					
                if(pop(machine).word.as_int != 0){
                    ip = machine->instructions.data[ip].value.as_int - 1;
                    if(ip + 1 >= machine->program_size){
	                    TIM_ERROR("error: cannot nzjmp out of bounds to: %ld\n", machine->instructions.data[ip].value.as_int);														
                    }
                } else {
                    break;
                }
                break;
            case INST_PRINT:
                a = pop(machine);
                printf("as float: %f, as int: %ld, as char: %c, as pointer: %p, type: %s\n",
                        a.word.as_float, a.word.as_int, a.word.as_char, a.word.as_pointer, str_types[a.type]);
                break;
            case INST_SS:
                push(machine, (Word){.as_int=machine->stack_size}, INT_TYPE);
                break;
            case INST_NATIVE: {
                machine->native_ptrs[machine->instructions.data[ip].value.as_int](machine);
            } break;
            case INST_ENTRYPOINT:
                assert(false);
                break;
            case INST_HALT:
                ip = machine->program_size;
                break;
			case INST_LOAD_LIBRARY: {
				char *lib_name = (char*)pop(machine).word.as_pointer;			
				char *func_name = (char*)pop(machine).word.as_pointer;		
				void *lib = dlopen(lib_name, RTLD_LAZY);		
				if(!lib) {
					fprintf(stderr, "error loading lib: %s\n", dlerror());
					exit(1);
				}
				native func;
				*(void**)(&func) = dlsym(lib, func_name);				
				machine_load_native(machine, func);
		// WIP FIX THIS
		/*
				while(func_name[0] != '\0') {
					native func;				
					*(void**)(&func) = dlsym(lib, func_name);							
					if(!func) {
						fprintf(stderr, "error loading function: %s\n", dlerror());
						exit(1);
					}
					machine_load_native(machine, func);
					func_name = (char*)pop(machine).word.as_pointer;						
				}
		*/
			} break;
            case INST_COUNT:
                assert(false);
        }
    }
	for(size_t i = 2; i < machine->native_ptrs_s; i++) {
		dlclose(machine->native_ptrs);
	}
}
