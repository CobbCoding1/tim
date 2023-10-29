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

int str_stack_size = 0;

Inst *generate_instructions(ParseList *head, int *program_size, char str_stack[MAX_STACK_SIZE][MAX_STRING_SIZE]){
    Inst *program = malloc(sizeof(Inst) * length_of_list(head));
    Inst_Set insts[INST_COUNT + 1] = {    
        INST_NOP, INST_PUSH, INST_PUSH_PTR, INST_PUSH_STR, INST_POP, INST_DUP, INST_INDUP, INST_SWAP, INST_INSWAP, INST_ADD, INST_SUB, INST_MUL, 
        INST_DIV, INST_MOD, INST_ADD_F, INST_SUB_F, INST_MUL_F, INST_DIV_F, INST_MOD_F, INST_CMPE, INST_CMPNE, INST_CMPG, 
        INST_CMPL, INST_CMPGE, INST_CMPLE, INST_JMP, INST_ZJMP, INST_NZJMP, INST_PRINT, INST_NATIVE, INST_HALT, INST_COUNT};

    while(head != NULL){
        assert(head->value.type != TYPE_NONE && "Value should not be none\n");
        assert(head->value.type < (TokenType)INST_COUNT);

        Inst *instruction = malloc(sizeof(Inst));
        instruction->type = insts[head->value.type];
        if(head->value.type == TYPE_INDUP || head->value.type == TYPE_INSWAP || head->value.type == TYPE_JMP || head->value.type == TYPE_ZJMP || head->value.type == TYPE_NZJMP){
            head = head->next;
            instruction->value.as_int = atoi(head->value.text);
        }

        if(head->value.type == TYPE_NATIVE){
            head = head->next;
            instruction->value.as_int = atoi(head->value.text);
        }

        if(head->value.type == TYPE_PUSH || head->value.type == TYPE_PUSH_STR){
            head = head->next;
            if(head->value.type == TYPE_INT){
                instruction->value.as_int = atoi(head->value.text);
            } else if(head->value.type == TYPE_FLOAT){
                instruction->value.as_float = atof(head->value.text);
            } else if(head->value.type == TYPE_CHAR){
                instruction->value.as_char = head->value.text[0];
            } else if(head->value.type == TYPE_STRING){
                strncpy(str_stack[str_stack_size++], head->value.text, MAX_STRING_SIZE - 1);
            } else {
                assert(false && "you should not be here\n");
            }
        }
        push_program(program, program_size, *instruction);
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
    Inst *program = generate_instructions(&list, &program_size, machine.str_stack);
    machine.instructions = program;
    machine.program_size = program_size;
    machine.str_stack_size = str_stack_size;
    write_program_to_file(&machine, output_file);
    return 0;
}
