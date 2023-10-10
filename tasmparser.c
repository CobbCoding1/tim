#include "tasmparser.h"

void append(ParseList *head, Token value){
    ParseList *tmp = head;
    while(tmp->next != NULL){
        tmp = tmp->next;
    }
    ParseList *new = malloc(sizeof(ParseList));
    new->value = value;
    new->next = NULL;
    tmp->next = new;
}

void print_list(ParseList *head){
    while(head != NULL){
        print_token(head->value);
        head = head->next;
    }
}

void generate_list(ParseList *root, Lexer *lexer){
    assert(root != NULL && "Root cannot equal NULL\n");
    for(int index = 1; index < lexer->stack_size; index++){
        switch(lexer->token_stack[index].type){
            case TYPE_NONE:
                assert(false && "Token should not be NONE\n");
                break;
            case TYPE_NOP:
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_PUSH:
                append(root, lexer->token_stack[index]);
                index++;
                if(lexer->token_stack[index].type != TYPE_INT) {
                    fprintf(stderr, "ERROR: Expected type INT but found %s\n", "TODO: Implement token print");
                    exit(1);
                }
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_POP:
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_DUP:
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_INDUP:
                append(root, lexer->token_stack[index]);
                index++;
                if(lexer->token_stack[index].type != TYPE_INT){
                    fprintf(stderr, "ERROR: Expected type INT but found %s\n", "TODO: Implement token print");
                    exit(1);
                }
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_SWAP:
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_INSWAP:
                append(root, lexer->token_stack[index]);
                index++;
                if(lexer->token_stack[index].type != TYPE_INT){
                    fprintf(stderr, "ERROR: Expected type INT but found %s\n", "TODO: Implement token print");
                    exit(1);
                }
                append(root, lexer->token_stack[index]);
                index++;
                break;
            case TYPE_ADD:
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_SUB:
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_MUL:
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_DIV:
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_MOD:
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_CMPE:
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_CMPNE:
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_CMPG:
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_CMPL:
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_CMPGE:
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_CMPLE:
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_JMP:
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_ZJMP:
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_NZJMP:
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_PRINT:
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_INT:
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_HALT:
                append(root, lexer->token_stack[index]);
                break;
            default:
                assert(false && "UNREACHABLE\n");
        }
    }
}

ParseList parser(Lexer lexer){
    ParseList root = {.value = lexer.token_stack[0], .next = NULL};
    generate_list(&root, &lexer);

    return root;
}
