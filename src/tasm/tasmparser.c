#include "tasmparser.h"

void append(ParseList *head, Token value){
    ParseList *new = malloc(sizeof(ParseList)); new->value = value;
    new->next = NULL;

    if(head == NULL){
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

void handle_token_def(Lexer *lexer, Token token, int index, int line_num, struct hashmap_s *hashmap){
    if (hashmap_remove(hashmap, lexer->token_stack[index].text, strlen(lexer->token_stack[index].text)) == 0) {
        fprintf(stderr, "%s:%d:%d: error: label '%s' already declared\n", token.file_name, lexer->token_stack[index].line, lexer->token_stack[index].character, lexer->token_stack[index].text);
        exit(1);
    }
    int *in = malloc(sizeof(int));
    *in = line_num;

    int hashmap_error = hashmap_put(hashmap, lexer->token_stack[index].text, strlen(lexer->token_stack[index].text), in);
    assert(hashmap_error == 0 && "could not place item in hashmap\n");

    lexer->token_stack[index].type = TYPE_NOP;
}

void print_syntax_error(Token *current_token, char *type_of_error, char *expected){
    fprintf(stderr, "%s:%d:%d %s error: Expected %s but found %s\n", current_token->file_name, current_token->line, current_token->character, 
            type_of_error, expected, pretty_token(*current_token));
    exit(1);
}

int expect_token(Lexer *lexer, int index, int count, ...){
	assert(index < lexer->stack_size);
    int result = 0;
    va_list list;
    va_start(list, count);
    for(int i = 0; i < count; i++){
        if(lexer->token_stack[index].type == va_arg(list, int)){
            result = 1;
        }
    }
    va_end(list);
    return result;
}

int check_if_register(TokenType type){
    if(type != TYPE_REGISTER){
        return 0;
    }
    return 1;
}

void generate_list(ParseList *root, Lexer *lexer, struct hashmap_s *hashmap){
    int line_num = 0;
    for(int index = 0; index < lexer->stack_size; index++){
        assert(lexer->token_stack[index].type != TYPE_NONE && "Should not be none\n");
        Token current_token = lexer->token_stack[index];

        if(expect_token(lexer, index, 6, TYPE_INT, TYPE_FLOAT, TYPE_CHAR, TYPE_LABEL, TYPE_STRING, TYPE_NULL)){
            print_syntax_error(&current_token, "unexpected token", "non-type");
        }
        if(lexer->token_stack[index].type == TYPE_INT && lexer->token_stack[index].type == TYPE_FLOAT && 
           lexer->token_stack[index].type == TYPE_CHAR && lexer->token_stack[index].type == TYPE_LABEL){
            index -= 2;
            continue;
        }

        if(expect_token(lexer, index, 1, TYPE_LABEL_DEF)){
            handle_token_def(lexer, current_token, index, line_num, hashmap);
        }

        append(root, lexer->token_stack[index]);

        if(expect_token(lexer, index, 5, TYPE_CALL, TYPE_JMP, TYPE_ZJMP, TYPE_NZJMP, TYPE_ENTRYPOINT)){
            index++;
            current_token = lexer->token_stack[index];
            if(lexer->token_stack[index].type != TYPE_INT && lexer->token_stack[index].type != TYPE_LABEL){
                print_syntax_error(&current_token, "syntax", "type of int or type of label");
            }
            append(root, lexer->token_stack[index]);
        }

        if(expect_token(lexer, index, 1, TYPE_PUSH)){
            index++;
            current_token = lexer->token_stack[index];
            if(lexer->token_stack[index].type != TYPE_INT && lexer->token_stack[index].type != TYPE_NULL &&
               lexer->token_stack[index].type != TYPE_CHAR && lexer->token_stack[index].type != TYPE_FLOAT && 
               !check_if_register(lexer->token_stack[index].type)
              ){
                print_syntax_error(&current_token, "syntax", "type of int or type of float or type of char");
            }
            append(root, lexer->token_stack[index]);
        }

        if(expect_token(lexer, index, 1, TYPE_PUSH_STR)){
            index++;
            current_token = lexer->token_stack[index];
            if(lexer->token_stack[index].type != TYPE_STRING){
                print_syntax_error(&current_token, "syntax", "type of int or type of float or type of char or type of string");
            }
            append(root, lexer->token_stack[index]);
        }

        if(expect_token(lexer, index, 1, TYPE_NATIVE)){
            index++;
            current_token = lexer->token_stack[index];
            if(!expect_token(lexer, index, 1, TYPE_INT)){
                print_syntax_error(&current_token, "syntax", "type of int");
            }
            append(root, lexer->token_stack[index]);
        }

        if(expect_token(lexer, index, 1, TYPE_MOV)){
            index++;
            current_token = lexer->token_stack[index];
            if(!check_if_register(current_token.type)){
                print_syntax_error(&current_token, "syntax", "a register");
            }
            append(root, lexer->token_stack[index]);
            index++;
            current_token = lexer->token_stack[index];
            if(lexer->token_stack[index].type != TYPE_INT && lexer->token_stack[index].type != TYPE_NULL &&
               lexer->token_stack[index].type != TYPE_CHAR && lexer->token_stack[index].type != TYPE_FLOAT && lexer->token_stack[index].type != TYPE_TOP){
                print_syntax_error(&current_token, "syntax", "type of int or type of float or type of char, or top of stack");
            }
            append(root, lexer->token_stack[index]);
        }
        line_num++;
    }
}

void check_labels(ParseList *head, Lexer *lexer, struct hashmap_s *hashmap){
    while(head != NULL){
        if(head->value.type == TYPE_LABEL){
            void *const label_index = hashmap_get(hashmap, head->value.text, strlen(head->value.text));
            if(label_index == NULL){
                fprintf(stderr, "%s:%d:%d: error: undeclared label: '%s'\n", lexer->file_name, head->value.line, head->value.character, head->value.text);
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

    int hashmap_error = hashmap_create(initial_size, &label_map);
    assert(hashmap_error == 0 && "could not initialize hashmap\n");

    ParseList root = {0};
    generate_list(&root, &lexer, &label_map);
    root = *root.next;
    check_labels(&root, &lexer, &label_map);
    //print_list(&root);

    return root;
}
