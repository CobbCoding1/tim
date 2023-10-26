
#include "tasmlexer.h"

#define TIPP_IMPLEMENTATION
#include "tipp.h"

char *open_file(char *file_path, int *length){
    FILE *file = fopen(file_path, "r");
    if(!file){
        fprintf(stderr, "error: could not find file: %s\n", file_path);
        exit(1);
    }
    char *current = {0};

    fseek(file, 0, SEEK_END);
    *length = ftell(file);
    fseek(file, 0, SEEK_SET);

    current = malloc(sizeof(char) * *length);
    fread(current, 1, *length, file);
    if(!current){
        fprintf(stderr, "error: could not read from file: %s\n", file_path);
        exit(1);
    }

    fclose(file);
    return current;
}

void push_token(Lexer *lexer, Token value){
    assert(lexer->stack_size < MAX_TOKEN_STACK_SIZE && "stack overflow\n");
    lexer->token_stack[lexer->stack_size++] = value;
}

Token pop_token(Lexer *lexer){
    assert(lexer->stack_size > 0 && "stack underflow\n");
    return lexer->token_stack[lexer->stack_size];
}

char *pretty_token(Token token){
    switch(token.type){
        case TYPE_NONE:
            return "none\n";
            break;
        case TYPE_NOP:
            return "nop\n";
            break;
        case TYPE_PUSH:
            return "push\n";
            break;
        case TYPE_POP:
            return "pop\n";
            break;
        case TYPE_DUP:
            return "dup\n";
            break;
        case TYPE_INDUP:
            return "indup\n";
            break;
        case TYPE_SWAP:
            return "swap\n";
            break;
        case TYPE_INSWAP:
            return "inswap\n";
            break;
        case TYPE_ADD:
            return "add\n";
            break;
        case TYPE_SUB:
            return "sub\n";
            break;
        case TYPE_MUL:
            return "mul\n";
            break;
        case TYPE_DIV:
            return "div\n";
            break;
        case TYPE_MOD:
            return "mod\n";
            break;
        case TYPE_ADD_F:
            return "addf\n";
            break;
        case TYPE_SUB_F:
            return "subf\n";
            break;
        case TYPE_MUL_F:
            return "mulf\n";
            break;
        case TYPE_DIV_F:
            return "divf\n";
            break;
        case TYPE_MOD_F:
            return "modf\n";
            break;
        case TYPE_CMPE:
            return "cmpe\n";
            break;
        case TYPE_CMPNE:
            return "cmpne\n";
            break;
        case TYPE_CMPG:
            return "cmpg\n";
            break;
        case TYPE_CMPL:
            return "cmpl\n";
            break;
        case TYPE_CMPGE:
            return "cmpge\n";
            break;
        case TYPE_CMPLE:
            return "cmple\n";
            break;
        case TYPE_JMP:
            return "jmp\n";
            break;
        case TYPE_ZJMP:
            return "zjmp\n";
            break;
        case TYPE_NZJMP:
            return "nzjmp\n";
            break;
        case TYPE_PRINT:
            return "print\n";
            break;
        case TYPE_WRITE:
            return "write\n";
            break;
        case TYPE_INT:
            return "type of int\n";
            break;
        case TYPE_FLOAT:
            return "type of float\n";
            break;
        case TYPE_CHAR:
            return "type of char\n";
            break;
        case TYPE_STRING:
            return "type of string\n";
            break;
        case TYPE_LABEL_DEF:
            return "label def\n";
            break;
        case TYPE_LABEL:
            return "type of label\n";
            break;
        case TYPE_HALT:
            return "halt\n";
            break;
        case TYPE_COUNT:
            return "count of types\n";
            break;
    }
    return "none\n";
}

void print_token(Token token){
    assert(&token != NULL && "error: token cannot be NULL\n");
    printf("%s\n", pretty_token(token));
    printf("text: %s, line: %d, character: %d\n", token.text, token.line, token.character);
}

Token init_token(TokenType type, char *text, int line, int character, char *file_name){
    Token token = {.type = type, .text = text, .line = line, .character = character, .file_name = file_name};
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
    } else if(strcmp(name, "write") == 0){
        return TYPE_WRITE;
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

Token generate_keyword(char *current, int *current_index, int *line, int *character, Lexer lex){
    char *keyword_name = malloc(sizeof(char) * 16);
    int keyword_length = 0;
    while(isalpha(current[*current_index]) || current[*current_index] == '_'){
        keyword_name[keyword_length] = current[*current_index];
        *current_index += 1;
        keyword_length++;
    }
    keyword_name[keyword_length] = '\0';
    TokenType type = check_builtin_keywords(keyword_name);
    if(type == TYPE_NONE){
        type = check_label_type(current, current_index);
    }
    Token token = init_token(type, keyword_name, *line, *character, lex.file_name);
    // Increment character by lenth of keyword, also subtract one because iteration occurs in lexer function as well
    *character += keyword_length - 1;
    return token;
}

Token generate_num(char *current, int *current_index, int line, int *character, Lexer lex){
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
        Token token = init_token(type, keyword_name, line, *character, lex.file_name);
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
    Token token = init_token(type, keyword_name, line, *character, lex.file_name);
    // Increment character by lenth of number, also subtract one because iteration occurs in lexer function as well
    *character += keyword_length - 1;
    return token;
}

char valid_escape_character(char *current, int *current_index){
    switch(current[*current_index]){
        case 'n':
            return '\n';
        case 't':
            return '\t';
        case 'v':
            return '\v';
        case 'b':
            return '\b';
        case 'r':
            return '\r';
        case 'f':
            return '\f';
        case 'a':
            return '\a';
        case '\\':
            return '\\';
        case '?':
            return '\?';
        case '\'':
            return '\'';
        case '\"':
            return '\"';
        case '0':
            return '\0';
        default:
            return ' ';
    }
}

Token generate_char(char *file_name, char *current, int *current_index, int line, int *character, Lexer lex){
    char *character_name = malloc(sizeof(char) * 2);
    *current_index += 1;
    character_name[0] = current[*current_index];
    if(current[*current_index] == '\\'){
        *current_index += 1;
        char escape_character = valid_escape_character(current, current_index);
        if(escape_character == ' '){
            fprintf(stderr, "%s:%d:%d error: invalid escape character\n", file_name, line, *character);
            exit(1);
        }
        character_name[0] = escape_character; 
    }
    *current_index += 1;
    if(current[*current_index] != '\''){
        fprintf(stderr, "%s:%d:%d error: expected close single quote\n", file_name, line, *character);
        exit(1);
    }

    character_name[1] = '\0';

    TokenType type = TYPE_CHAR;
    Token token = init_token(type, character_name, line, *character, lex.file_name);
    // Increment character by 3 because of the character length, also subtract one because iteration occurs in lexer function as well
    *character += (3) - 1;
    return token;
}

Token generate_string(char *file_name, char *current, int *current_index, int line, int *character, Lexer lex){
    char *string_name = malloc(sizeof(char) * 64);
    int string_index = 0;
    *current_index += 1;
    if(current[*current_index] == '\0'){
        fprintf(stderr, "%s:%d:%d error: invalid string\n", file_name, line, *character);
        exit(1);
    }

    while(current[*current_index] != '"' && current[*current_index] != '\0'){
        if(current[*current_index] == '\\'){
            *current_index += 1;
            char escape_character = valid_escape_character(current, current_index);
            if(escape_character == ' '){
                fprintf(stderr, "%s:%d:%d error: invalid escape character\n", file_name, line, *character);
                exit(1);
            }
            current[*current_index] = escape_character;
        }
        string_name[string_index] = current[*current_index];
        *current_index += 1;
        string_index++;
    }

    if(current[*current_index] != '"'){
        fprintf(stderr, "%s:%d:%d error: expected close quote\n", file_name, line, *character);
        exit(1);
    }

    string_name[string_index] = '\0';


    TokenType type = TYPE_STRING;
    Token token = init_token(type, string_name, line, *character, lex.file_name);
    // Increment character by 3 because of the character length, also subtract one because iteration occurs in lexer function as well
    *character += (string_index) - 1;
    return token;
}

Lexer lexer(char *file_name){
    int length = 0;
    char *current = prepro(file_name, &length, 0);

    int current_index = 0;
    int line = 1;
    int character = 1;

    Lexer lex = {.stack_size = 0, .file_name = file_name};

    while(current_index < length){
        if(current[current_index] == '\n'){
            line++;
            character = 0;
        }

        if(isalpha(current[current_index]) || current[current_index] == '_'){
            Token token = generate_keyword(current, &current_index, &line, &character, lex);
            push_token(&lex, token);
            current_index--;
        } else if(isdigit(current[current_index])){
            Token token = generate_num(current, &current_index, line, &character, lex);
            push_token(&lex, token);
            current_index--;
        } else if(current[current_index] == '\''){
            Token token = generate_char(file_name, current, &current_index, line, &character, lex);
            push_token(&lex, token);
        } else if(current[current_index] == '"'){
            Token token = generate_string(file_name, current, &current_index, line, &character, lex);
            push_token(&lex, token);
        } else if(current[current_index] == ';'){
            while(current[current_index] != '\n'){
                current_index++;
            }
            line++;
        } else if(current[current_index] == '@'){
            current_index++;
            if(current[current_index] != '"' || current_index > length){
                fprintf(stderr, "error: expected open paren\n");
                exit(1);
            }
            current_index++;
            if(current_index > length){
                fprintf(stderr, "error: expected file path\n");
                exit(1);
            }
            char *defined_file_path = get_filename(current, &current_index, length);
            lex.file_name = defined_file_path;
            
            current_index++;
            if(current_index > length){
                fprintf(stderr, "error: expected file path\n");
                exit(1);
            }
            current_index++;
            if(!isdigit(current[current_index]) || current_index > length){
                fprintf(stderr, "error: expected file path\n");
                exit(1);
            }
            Token num = generate_num(current, &current_index, line, &character, lex);
            current_index--;
            line = atoi(num.text);
            character = 0;
        } 
        character++;
        current_index++;
    }

    for(int i = 0; i < lex.stack_size; i++){
        //print_token(lex.token_stack[i]);
    }
    return lex;
}
