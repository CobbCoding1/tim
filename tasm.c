#include "tasm.h"

#define MAX_PROGRAM_SIZE 1024

Inst *generate_instructions(ParseList *head, Inst program[]){
    Inst instruction = {0};
    int program_size = 0;
    while(head != NULL){
        switch(head->value.type){
            case TYPE_NONE:
                assert(false && "Token should not be NONE\n");
                break;
            case TYPE_NOP:
                instruction.type = INST_NOP;
                program[program_size] = instruction;
                break;
            case TYPE_PUSH:
                head = head->next;
                instruction.type = INST_PUSH;
                instruction.value = atoi(head->value.text);
                program[program_size] = instruction;
                break;
            case TYPE_POP:
                instruction.type = INST_POP;
                program[program_size] = instruction;
                break;
            case TYPE_DUP:
                instruction.type = INST_DUP;
                program[program_size] = instruction;
                break;
            case TYPE_INDUP:
                head = head->next;
                instruction.type = INST_INDUP;
                instruction.value = atoi(head->value.text);
                program[program_size] = instruction;
                break;
                break;
            case TYPE_SWAP:
                instruction.type = INST_SWAP;
                program[program_size] = instruction;
                break;
            case TYPE_INSWAP:
                head = head->next;
                instruction.type = INST_INSWAP;
                instruction.value = atoi(head->value.text);
                program[program_size] = instruction;
                break;
                break;
            case TYPE_ADD:
                instruction.type = INST_ADD;
                program[program_size] = instruction;
                break;
            case TYPE_SUB:
                instruction.type = INST_SUB;
                program[program_size] = instruction;
                break;
            case TYPE_MUL:
                instruction.type = INST_MUL;
                program[program_size] = instruction;
                break;
            case TYPE_DIV:
                instruction.type = INST_DIV;
                program[program_size] = instruction;
                break;
            case TYPE_MOD:
                instruction.type = INST_MOD;
                program[program_size] = instruction;
                break;
            case TYPE_CMPE:
                instruction.type = INST_CMPE;
                program[program_size] = instruction;
                break;
            case TYPE_CMPNE:
                instruction.type = INST_CMPNE;
                program[program_size] = instruction;
                break;
            case TYPE_CMPG:
                instruction.type = INST_CMPG;
                program[program_size] = instruction;
                break;
            case TYPE_CMPL:
                instruction.type = INST_CMPL;
                program[program_size] = instruction;
                break;
            case TYPE_CMPGE:
                instruction.type = INST_CMPGE;
                program[program_size] = instruction;
                break;
            case TYPE_CMPLE:
                instruction.type = INST_CMPLE;
                program[program_size] = instruction;
                break;
            case TYPE_JMP:
                head = head->next;
                instruction.type = INST_JMP;
                instruction.value = atoi(head->value.text);
                program[program_size] = instruction;
                break;
            case TYPE_ZJMP:
                head = head->next;
                instruction.type = INST_NZJMP;
                instruction.value = atoi(head->value.text);
                program[program_size] = instruction;
                break;
            case TYPE_NZJMP:
                head = head->next;
                instruction.type = INST_NZJMP;
                instruction.value = atoi(head->value.text);
                program[program_size] = instruction;
                break;
            case TYPE_PRINT:
                instruction.type = INST_PRINT;
                program[program_size] = instruction;
                break;
            case TYPE_INT:
                assert(false && "ERROR: Should not be INT\n");
                break;
            case TYPE_HALT:
                instruction.type = INST_HALT;
                program[program_size] = instruction;
                break;
        }
        head = head->next;
        program_size++;
    }
    return program;
}

int main(int argc, char *argv[]){
    if(argc < 2){
        fprintf(stderr, "Usage: %s <file_name.tasm>\n", argv[0]);
        exit(1);
    }

    char *file_name = argv[1];
    Inst program[MAX_PROGRAM_SIZE] = {0};
    Lexer lex = lexer(file_name);
    ParseList list = parser(lex);
    print_list(&list);
    generate_instructions(&list, program);
    Machine machine = {.instructions = program, .program_size = sizeof(program)/sizeof(program[0])};
    write_program_to_file(&machine, "machine.tim");
    return 0;
}
