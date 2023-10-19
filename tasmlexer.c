
#include "tasmlexer.h"

char *open_file(char *file_path, int *length){
    FILE *file = fopen(file_path, "r");
    if(!file){
        fprintf(stderr, "ERROR: Could not open file %s\n", file_path);
        exit(1);
    }
    char *current = {0};

    fseek(file, 0, SEEK_END);
    *length = ftell(file);
    fseek(file, 0, SEEK_SET);

    current = malloc(sizeof(char) * *length);
    fread(current, 1, *length, file);
    if(!current){
        fprintf(stderr, "ERROR: Could not read from file\n");
    }

    fclose(file);
    return current;
}

void push_token(Lexer *lexer, Token value){
     if(lexer->stack_size >= MAX_TOKEN_STACK_SIZE){
        fprintf(stderr, "ERROR: Stack Overflow\n");
        exit(1);
    }
    lexer->token_stack[lexer->stack_size++] = value;
}

Token pop_token(Lexer *lexer){
     if(lexer->stack_size <= 0){
        fprintf(stderr, "ERROR: Stack Underflow\n");
        exit(1);
    }
    return lexer->token_stack[lexer->stack_size];
}

char *pretty_token(Token token){
    switch(token.type){
        case TYPE_NONE:
            return "TYPE NONE\n";
            break;
        case TYPE_NOP:
            return "TYPE NOP\n";
            break;
        case TYPE_PUSH:
            return "TYPE PUSH\n";
            break;
        case TYPE_POP:
            return "TYPE POP\n";
            break;
        case TYPE_DUP:
            return "TYPE DUP\n";
            break;
        case TYPE_INDUP:
            return "TYPE INDUP\n";
            break;
        case TYPE_SWAP:
            return "TYPE SWAP\n";
            break;
        case TYPE_INSWAP:
            return "TYPE INSWAP\n";
            break;
        case TYPE_ADD:
            return "TYPE ADD\n";
            break;
        case TYPE_SUB:
            return "TYPE SUB\n";
            break;
        case TYPE_MUL:
            return "TYPE MUL\n";
            break;
        case TYPE_DIV:
            return "TYPE DIV\n";
            break;
        case TYPE_MOD:
            return "TYPE MOD\n";
            break;
        case TYPE_ADD_F:
            return "TYPE ADD F\n";
            break;
        case TYPE_SUB_F:
            return "TYPE SUB F\n";
            break;
        case TYPE_MUL_F:
            return "TYPE MUL F\n";
            break;
        case TYPE_DIV_F:
            return "TYPE DIV F\n";
            break;
        case TYPE_MOD_F:
            return "TYPE MOD F\n";
            break;
        case TYPE_CMPE:
            return "TYPE CMPE\n";
            break;
        case TYPE_CMPNE:
            return "TYPE CMPNE\n";
            break;
        case TYPE_CMPG:
            return "TYPE CMPG\n";
            break;
        case TYPE_CMPL:
            return "TYPE CMPL\n";
            break;
        case TYPE_CMPGE:
            return "TYPE CMPGE\n";
            break;
        case TYPE_CMPLE:
            return "TYPE CMPLE\n";
            break;
        case TYPE_JMP:
            return "TYPE JMP\n";
            break;
        case TYPE_ZJMP:
            return "TYPE ZJMP\n";
            break;
        case TYPE_NZJMP:
            return "TYPE NZJMP\n";
            break;
        case TYPE_PRINT:
            return "TYPE PRINT\n";
            break;
        case TYPE_INT:
            return "TYPE INT\n";
            break;
        case TYPE_FLOAT:
            return "TYPE FLOAT\n";
            break;
        case TYPE_CHAR:
            return "TYPE CHAR\n";
            break;
        case TYPE_LABEL_DEF:
            return "TYPE LABEL DEF\n";
            break;
        case TYPE_LABEL:
            return "TYPE LABEL\n";
            break;
        case TYPE_HALT:
            return "TYPE HALT\n";
            break;
        case TYPE_COUNT:
            return "TYPE COUNT";
            break;
    }
    return "TYPE NONE\n";
}

void print_token(Token token){
    assert(&token != NULL && "ERROR: Token cannot be NULL\n");
    printf("%s\n", pretty_token(token));
    printf("text: %s, line: %d, character: %d\n", token.text, token.line, token.character);
}

Token init_token(TokenType type, char *text, int line, int character){
    Token token = {.type = type, .text = text, .line = line, .character = character};
    return token;
}

TokenType check_builtin_keywords(char *name){
    if(strcmp(name, "nop") == 0){
        return TYPE_NOP;
    } else if(strcmp(name, "push") == 0){
        return TYPE_PUSH;
    } else if(strcmp(name, "pop") == 0){
        return TYPE_POP;
    } else if(strcmp(name, "dup") == 0){
        return TYPE_DUP;
    } else if(strcmp(name, "indup") == 0){
        return TYPE_INDUP;
    } else if(strcmp(name, "swap") == 0){
        return TYPE_SWAP;
    } else if(strcmp(name, "inswap") == 0){
        return TYPE_INSWAP;
    } else if(strcmp(name, "add") == 0){
        return TYPE_ADD;
    } else if(strcmp(name, "sub") == 0){
        return TYPE_SUB;
    } else if(strcmp(name, "mul") == 0){
        return TYPE_MUL;
    } else if(strcmp(name, "div") == 0){
        return TYPE_DIV;
    } else if(strcmp(name, "mod") == 0){
        return TYPE_MOD;
    } else if(strcmp(name, "addf") == 0){
        return TYPE_ADD_F;
    } else if(strcmp(name, "subf") == 0){
        return TYPE_SUB_F;
    } else if(strcmp(name, "mulf") == 0){
        return TYPE_MUL_F;
    } else if(strcmp(name, "divf") == 0){
        return TYPE_DIV_F;
    } else if(strcmp(name, "modf") == 0){
        return TYPE_MOD_F;
    } else if(strcmp(name, "cmpe") == 0){
        return TYPE_CMPE;
    } else if(strcmp(name, "cmpne") == 0){
        return TYPE_CMPNE;
    } else if(strcmp(name, "cmpg") == 0){
        return TYPE_CMPG;
    } else if(strcmp(name, "cmpl") == 0){
        return TYPE_CMPL;
    } else if(strcmp(name, "cmpge") == 0){
        return TYPE_CMPGE;
    } else if(strcmp(name, "cmple") == 0){
        return TYPE_CMPLE;
    } else if(strcmp(name, "jmp") == 0){
        return TYPE_JMP;
    } else if(strcmp(name, "zjmp") == 0){
        return TYPE_ZJMP;
    } else if(strcmp(name, "nzjmp") == 0){
        return TYPE_NZJMP;
    } else if(strcmp(name, "print") == 0){
        return TYPE_PRINT;
    } else if(strcmp(name, "halt") == 0){
        return TYPE_HALT;
    }
    return TYPE_NONE;
}    

TokenType check_label_type(char *current, int *current_index){
    if(current[*current_index] == ':'){
        return TYPE_LABEL_DEF;
    }
    return TYPE_LABEL;
}

Token generate_keyword(char *current, int *current_index, int line, int character){
    char *keyword_name = malloc(sizeof(char) * 16);
    int keyword_length = 0;
    while(isalpha(current[*current_index])){
        keyword_name[keyword_length] = current[*current_index];
        *current_index += 1;
        keyword_length++;
    }
    keyword_name[keyword_length] = '\0';
    TokenType type = check_builtin_keywords(keyword_name);
    if(type == TYPE_NONE){
        type = check_label_type(current, current_index);
    }
    Token token = init_token(type, keyword_name, line, character);
    return token;
}

Token generate_num(char *current, int *current_index, int line, int character){
    char *keyword_name = malloc(sizeof(char) * 16);
    int keyword_length = 0;
    while(isdigit(current[*current_index])){
        keyword_name[keyword_length] = current[*current_index];
        *current_index += 1;
        keyword_length++;
    }
    if(current[*current_index] != '.'){
        keyword_name[keyword_length] = '\0';
        TokenType type = TYPE_INT;
        Token token = init_token(type, keyword_name, line, character);
        return token;
    }
    keyword_name[keyword_length] = current[*current_index];
    *current_index += 1;
    keyword_length++;
    while(isdigit(current[*current_index])){
        keyword_name[keyword_length] = current[*current_index];
        *current_index += 1;
        keyword_length++;
    }
    keyword_name[keyword_length] = '\0';
    TokenType type = TYPE_FLOAT;
    Token token = init_token(type, keyword_name, line, character);
    return token;
}

Token generate_char(char *current, int *current_index, int line, int character){
    char *character_name = malloc(sizeof(char) * 2);
    *current_index += 1;
    character_name[0] = current[*current_index];
    *current_index += 1;
    if(current[*current_index] != '\''){
        fprintf(stderr, "ERROR: Expected close single quote\n");
        exit(1);
    }
    character_name[1] = '\0';
    TokenType type = TYPE_CHAR;
    Token token = init_token(type, character_name, line, character);
    return token;
}

Lexer lexer(char *file_name){
    int length;
    char *current = open_file(file_name, &length);
    int current_index = 0;

    int line = 1;
    int character = 1;

    Lexer lex = {.stack_size = 0, .file_name = "test.tasm"};

    while(current_index < length){
        if(current[current_index] == '\n'){
            line++;
            character = 0;
        }

        if(isalpha(current[current_index])){
            Token token = generate_keyword(current, &current_index, line, character);
            push_token(&lex, token);
            current_index--;
        } else if(isdigit(current[current_index])){
            Token token = generate_num(current, &current_index, line, character);
            push_token(&lex, token);
            current_index--;
        } else if(current[current_index] == '\''){
            Token token = generate_char(current, &current_index, line, character);
            push_token(&lex, token);
        } else if(current[current_index] == ';'){
            while(current[current_index] != '\n'){
                current_index++;
            }
            line++;
        } 
        character++;
        current_index++;
    }

    for(int i = 0; i < lex.stack_size; i++){
        //print_token(lex.token_stack[i]);
    }
    return lex;
}
