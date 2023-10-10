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

Inst *generate_instructions(ParseList *head, int *program_size){
    Inst *program = malloc(sizeof(Inst));
    while(head != NULL){
        Inst *instruction = malloc(sizeof(Inst));
        switch(head->value.type){
            case TYPE_NONE:
                assert(false && "Token should not be NONE\n");
                break;
            case TYPE_NOP:
                instruction->type = INST_NOP;
                break;
            case TYPE_PUSH:
                head = head->next;
                instruction->type = INST_PUSH;
                instruction->value = atoi(head->value.text);
                break;
            case TYPE_POP:
                instruction->type = INST_POP;
                break;
            case TYPE_DUP:
                instruction->type = INST_DUP;
                break;
            case TYPE_INDUP:
                head = head->next;
                instruction->type = INST_INDUP;
                instruction->value = atoi(head->value.text);
                break;
                break;
            case TYPE_SWAP:
                instruction->type = INST_SWAP;
                break;
            case TYPE_INSWAP:
                head = head->next;
                instruction->type = INST_INSWAP;
                instruction->value = atoi(head->value.text);
                break;
                break;
            case TYPE_ADD:
                instruction->type = INST_ADD;
                break;
            case TYPE_SUB:
                instruction->type = INST_SUB;
                break;
            case TYPE_MUL:
                instruction->type = INST_MUL;
                break;
            case TYPE_DIV:
                instruction->type = INST_DIV;
                break;
            case TYPE_MOD:
                instruction->type = INST_MOD;
                break;
            case TYPE_CMPE:
                instruction->type = INST_CMPE;
                break;
            case TYPE_CMPNE:
                instruction->type = INST_CMPNE;
                break;
            case TYPE_CMPG:
                instruction->type = INST_CMPG;
                break;
            case TYPE_CMPL:
                instruction->type = INST_CMPL;
                break;
            case TYPE_CMPGE:
                instruction->type = INST_CMPGE;
                break;
            case TYPE_CMPLE:
                instruction->type = INST_CMPLE;
                break;
            case TYPE_JMP:
                head = head->next;
                instruction->type = INST_JMP;
                instruction->value = atoi(head->value.text);
                break;
            case TYPE_ZJMP:
                head = head->next;
                instruction->type = INST_ZJMP;
                instruction->value = atoi(head->value.text);
                break;
            case TYPE_NZJMP:
                head = head->next;
                instruction->type = INST_NZJMP;
                instruction->value = atoi(head->value.text);
                break;
            case TYPE_PRINT:
                instruction->type = INST_PRINT;
                break;
            case TYPE_INT:
                assert(false && "ERROR: Should not be INT\n");
                break;
            case TYPE_HALT:
                instruction->type = INST_HALT;
                break;
        }
        push_program(program, program_size, *instruction);
        head = head->next;
    }
    return program;
}

int main(int argc, char *argv[]){
    if(argc < 2){
        fprintf(stderr, "Usage: %s <file_name.tasm>\n", argv[0]);
        exit(1);
    }

    char *file_name = argv[1];
    Lexer lex = lexer(file_name);
    ParseList list = parser(lex);
    //print_list(&list);
    int program_size = 0;
    Inst *program = generate_instructions(&list, &program_size);
    Machine machine = {.instructions = program, .program_size = program_size};
    write_program_to_file(&machine, "machine.tim");
    return 0;
}
