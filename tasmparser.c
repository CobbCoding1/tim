#include "tasmparser.h"

void append(ParseList *head, Token value){
    ParseList *new = malloc(sizeof(ParseList));
    new->value = value;
    new->next = NULL;

    ParseList *tmp = head;
    while(tmp->next != NULL){
        tmp = tmp->next;
    }
    tmp->next = new;
}

void print_list(ParseList *head){
    while(head != NULL){
        print_token(head->value);
        head = head->next;
    }
}

void handle_token_def(Lexer *lexer, int index, int line_num, struct hashmap_s *hashmap){
    if (hashmap_remove(hashmap, lexer->token_stack[index].text, strlen(lexer->token_stack[index].text)) == 0) {
        printf("ERROR: Redeclaration of label %s\n", lexer->token_stack[index].text);
    }
    int *in = malloc(sizeof(int));
    *in = line_num;

    if(hashmap_put(hashmap, lexer->token_stack[index].text, strlen(lexer->token_stack[index].text), in) != 0){
        fprintf(stderr, "ERROR: Could not place %s in the hashmap\n", lexer->token_stack[index].text);
        exit(1);
    }

    lexer->token_stack[index].type = TYPE_NOP;
}

void generate_list(ParseList *root, Lexer *lexer, struct hashmap_s *hashmap){
    int line_num = 0;
    for(int index = 1; index < lexer->stack_size; index++){
        //print_token(lexer->token_stack[index]);
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
                index++;
                if(lexer->token_stack[index].type != TYPE_INT && lexer->token_stack[index].type != TYPE_LABEL){
                    fprintf(stderr, "ERROR: Expected type INT OR LABEL but found %s\n", "TODO: Implement token print");
                    exit(1);
                }
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_ZJMP:
                append(root, lexer->token_stack[index]);
                index++;
                if(lexer->token_stack[index].type != TYPE_INT && lexer->token_stack[index].type != TYPE_LABEL){
                    fprintf(stderr, "ERROR: Expected type INT OR LABEL but found %s\n", "TODO: Implement token print");
                    exit(1);
                }
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_NZJMP:
                append(root, lexer->token_stack[index]);
                index++;
                if(lexer->token_stack[index].type != TYPE_INT && lexer->token_stack[index].type != TYPE_LABEL){
                    fprintf(stderr, "ERROR: Expected type INT OR LABEL but found %s\n", "TODO: Implement token print");
                    exit(1);
                }
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_PRINT:
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_INT:
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_LABEL_DEF:
                handle_token_def(lexer, index, line_num, hashmap);
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_LABEL:
                append(root, lexer->token_stack[index]);
                break;
            case TYPE_HALT:
                append(root, lexer->token_stack[index]);
                break;
            default:
                assert(false && "UNREACHABLE\n");
        }
        line_num++;
    }
}

void check_labels(ParseList *head, struct hashmap_s *hashmap){
    while(head != NULL){
        if(head->value.type == TYPE_LABEL){
            void *const label_index = hashmap_get(hashmap, head->value.text, strlen(head->value.text));
            if(label_index == NULL){
                fprintf(stderr, "ERROR: Undeclared label: %s\n", head->value.text);
                exit(1);
            }
            head->value.type = TYPE_INT;
            sprintf(head->value.text, "%d", *(int*)label_index);
        }
        head = head->next;
    }
}

ParseList parser(Lexer lexer){

    const unsigned initial_size = 2;
    struct hashmap_s label_map;
    if(hashmap_create(initial_size, &label_map) != 0){
        fprintf(stderr, "ERROR: Could not initialize hashmap\n");
        exit(1);
    }

    if(lexer.token_stack[0].type == TYPE_LABEL_DEF){
        handle_token_def(&lexer, 0, 0, &label_map);
    }
    ParseList root = {.value = lexer.token_stack[0], .next = NULL};
    generate_list(&root, &lexer, &label_map);
    check_labels(&root, &label_map);
    //print_list(&root);

    return root;
}
