#include "tasmparser.h"

void append(ParseList *head, Token value){
    ParseList *new = malloc(sizeof(ParseList));
    new->value = value;
    new->next = NULL;

    if(head == NULL){
        printf("ASDHKJL\n");
        head = new;
        return;
    }

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
    for(int index = 0; index < lexer->stack_size; index++){
        assert(lexer->token_stack[index].type != TYPE_NONE && "Should not be none\n");
        if(lexer->token_stack[index].type == TYPE_INT && lexer->token_stack[index].type == TYPE_FLOAT && 
           lexer->token_stack[index].type == TYPE_CHAR && lexer->token_stack[index].type == TYPE_LABEL){
            index -= 2;
            continue;
        }

        if(lexer->token_stack[index].type == TYPE_LABEL_DEF){
            handle_token_def(lexer, index, line_num, hashmap);
        }
        append(root, lexer->token_stack[index]);

        if(lexer->token_stack[index].type == TYPE_INDUP || lexer->token_stack[index].type == TYPE_INSWAP || 
           lexer->token_stack[index].type == TYPE_JMP || lexer->token_stack[index].type == TYPE_ZJMP || 
           lexer->token_stack[index].type == TYPE_NZJMP){
            index++;
            if(lexer->token_stack[index].type != TYPE_INT && lexer->token_stack[index].type != TYPE_LABEL){
                fprintf(stderr, "ERROR: Expected type INT but found %s\n", pretty_token(lexer->token_stack[index]));
                exit(1);
            }
            append(root, lexer->token_stack[index]);
        }

        if(lexer->token_stack[index].type == TYPE_PUSH){
            index++;
            if(lexer->token_stack[index].type != TYPE_INT && lexer->token_stack[index].type != TYPE_FLOAT && 
               lexer->token_stack[index].type != TYPE_CHAR && lexer->token_stack[index].type != TYPE_LABEL){
                fprintf(stderr, "ERROR: Expected type INT but found %s\n", pretty_token(lexer->token_stack[index]));
                exit(1);
            }
            append(root, lexer->token_stack[index]);
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
    ParseList root = {0};
    generate_list(&root, &lexer, &label_map);
    root = *root.next;
    check_labels(&root, &label_map);
    //print_list(&root);

    return root;
}
