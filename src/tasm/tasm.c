#include "tasm.h"

#define MAX_PROGRAM_SIZE 1024

void push_program(Inst program[], int *program_size, Inst value){
    assert(*program_size < MAX_PROGRAM_SIZE && "Program size cannot exceed max program size\n");
    program[(*program_size)++] = value;
}

Inst pop_program(Inst program[], int program_size){
    assert(program_size > 0 && "STACK UNDERFLOW\n");
    return program[--program_size];
}

size_t length_of_list(ParseList *head){
    ParseList *tmp = head;
    size_t result = 1;
    while(tmp->next != NULL){
        result += 1;
        tmp = tmp->next;
    }
    return result;
}

char *remove_first_character(char *str){
    int length = strlen(str);
    char *result = malloc(sizeof(char) * length);
    for(int i = 0; i < length - 1; i++){
        result[i] = str[i + 1];
    }
    result[length - 1] = '\0';
    return result;
}

size_t get_register_index(ParseList *head){
    size_t result = (size_t)atoi(remove_first_character(head->value.text));
    if(result >= AMOUNT_OF_REGISTERS){
        fprintf(stderr, "error: register index is too great\n");
        exit(1);
    }
    return result;
}

Inst *generate_instructions(ParseList *head, int *program_size, String_View str_stack[MAX_STACK_SIZE], size_t *entrypoint, bool *has_entrypoint){
    Inst *program = malloc(sizeof(Inst) * length_of_list(head));
    Inst_Set insts[INST_COUNT + 1] = {    
        INST_NOP, INST_PUSH, INST_PUSH_STR, INST_MOV, INST_REF, INST_DEREF, 
        INST_ALLOC, INST_DEALLOC, INST_WRITE, INST_READ, INST_POP, INST_DUP, INST_INDUP, INST_SWAP, INST_INSWAP, 
		INST_ADD, INST_SUB, INST_MUL, INST_DIV, INST_MOD, INST_ADD_F, INST_SUB_F, 
        INST_MUL_F, INST_DIV_F, INST_MOD_F, INST_CMPE, 
        INST_CMPNE, INST_CMPG, INST_CMPL, INST_CMPGE, INST_CMPLE, INST_ITOF, INST_FTOI, INST_ITOC, 
        INST_TOI, INST_TOF, INST_TOC, INST_TOVP, INST_CALL, INST_RET, 
        INST_JMP, INST_ZJMP, INST_NZJMP, INST_PRINT, INST_NATIVE, INST_ENTRYPOINT, INST_SS,
        INST_HALT, INST_COUNT
    };

    while(head != NULL){
        assert(head->value.type != TYPE_NONE && "Value should not be none\n");
        assert(head->value.type < (TokenType)INST_COUNT && "Incorrect value\n");
        Inst *instruction = malloc(sizeof(Inst));
        instruction->type = insts[head->value.type];
        if(
                head->value.type == TYPE_CALL || head->value.type == TYPE_NATIVE || 
                head->value.type == TYPE_JMP || head->value.type == TYPE_ZJMP || 
                head->value.type == TYPE_NZJMP
        ){
            head = head->next;
            instruction->value.as_int = atoi(head->value.text);
            instruction->data_type = INT_TYPE;
        }

        if(head->value.type == TYPE_ENTRYPOINT){
            instruction->type = INST_NOP;
            head = head->next;
            if(!*has_entrypoint){
                *entrypoint = (size_t)atoi(head->value.text);
                *has_entrypoint = true;
            } else {
                fprintf(stderr, "error: cannot define entrypoint more than once\n");
                exit(1);
            }
        }


        if(head->value.type == TYPE_PUSH){
            head = head->next;
            if(head->value.type == TYPE_INT){
                instruction->value.as_int = atoi(head->value.text);
                instruction->data_type = INT_TYPE;
            } else if(head->value.type == TYPE_FLOAT){
                instruction->value.as_float = atof(head->value.text);
                instruction->data_type = FLOAT_TYPE;
            } else if(head->value.type == TYPE_CHAR){
                instruction->value.as_char = head->value.text[0];
                instruction->data_type = CHAR_TYPE;
            } else if(check_if_register(head->value.type)){
                instruction->register_index = get_register_index(head);             
                instruction->data_type = REGISTER_TYPE;
            } else {
                assert(false && "you should not be here\n");
            }
        }

        if(head->value.type == TYPE_MOV){
            head = head->next;
            instruction->register_index = get_register_index(head);             
            head = head->next;
            if(head->value.type == TYPE_INT){
                instruction->value.as_int = atoi(head->value.text);
                instruction->data_type = INT_TYPE;
            } else if(head->value.type == TYPE_FLOAT){
                instruction->value.as_float = atof(head->value.text);
                instruction->data_type = FLOAT_TYPE;
            } else if(head->value.type == TYPE_CHAR){
                instruction->value.as_char = head->value.text[0];
                instruction->data_type = CHAR_TYPE;
            } else if(head->value.type == TYPE_TOP){
                instruction->data_type = TOP_TYPE;
            } else {
                assert(false && "you should not be here\n");
            }
        }

        if(head->value.type == TYPE_PUSH_STR){
            head = head->next;
            if(head->value.type != TYPE_STRING){
                assert(false && "why are you here\n");
            }
            instruction->type = INST_PUSH_STR;
            instruction->value.as_int = str_stack_size;
            instruction->data_type = INT_TYPE;
            size_t str_s = strlen(head->value.text)+1;
            str_stack[str_stack_size].len = str_s;
            str_stack[str_stack_size].data = malloc(sizeof(char)*str_s);
            strncpy(str_stack[str_stack_size++].data, head->value.text, str_s);
        }

        push_program(program, program_size, *instruction);
        free(instruction);
        head = head->next;
    }
    return program;
}

char *chop_file_by_dot(char *file_name){
    int index;
    char *result = malloc(sizeof(char) * 64);
    for(index = 0; file_name[index] != '.' && file_name[index] != '\0'; index++){
        result[index] = file_name[index];
    }
    snprintf(result + index, 5, ".tim");
    result = realloc(result, sizeof(char) * index);
    return result;
}

int main(int argc, char *argv[]){
    if(argc < 2){
        fprintf(stderr, "Usage: %s <file_name.tasm>\n", argv[0]);
        exit(1);
    }

    char *file_name = argv[1];
    char *output_file = chop_file_by_dot(file_name);
    Lexer lex = lexer(file_name);
    ParseList list = parser(lex);
    //print_list(&list);
    int program_size = 0;
    Machine machine;
    size_t entrypoint = 0;
    bool has_entrypoint = false;
    Inst *program = generate_instructions(&list, &program_size, machine.str_stack, &entrypoint, &has_entrypoint);
    machine.instructions = program;
    machine.program_size = program_size;
    machine.entrypoint = entrypoint;
    machine.has_entrypoint = has_entrypoint;
    machine.str_stack_size = str_stack_size;
    write_program_to_file(&machine, output_file);
    return 0;
}
