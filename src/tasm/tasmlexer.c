#include "tasmlexer.h"

#define TIPP_IMPLEMENTATION
#include "tipp.h"

int is_name(char character){
    if(isalpha(character) || character == '_'){
        return 1;
    }
    return 0;
}

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

char *token_type_text[TYPE_COUNT + 1] = {
    "nop","push","push_str","mov","ref","deref",
    "alloc", "dealloc", "write", "read", "pop","dup",
    "indup","swap","inswap","add","sub", "mul","div", "mod","add_f","sub_f","mul_f","div_f",
    "mod_f","cmpe","cmpne","cmpg","cmpl","cmpge","cmple","itof","ftoi","itoc","toi","tof","toc","tovp","call","ret","jmp","zjmp","nzjmp","print", 
    "native","entrypoint","ss","halt","int","float","char","string","NULL","register","label_def","label","top","count"
};

char *pretty_token(Token token){
    if(token.type > TYPE_COUNT){
        return "none";
    }
    return(token_type_text[token.type]);
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
    for(int i = 0; i < TYPE_COUNT; i++){
        if(strcmp(name, token_type_text[i]) == 0){
            return (TokenType)i;
        }
    }
    return TYPE_NONE;
}    

TokenType check_label_type(char *current, int *current_index){
    if(current[*current_index] == ':'){
        return TYPE_LABEL_DEF;
    }
    return TYPE_LABEL;
}

TokenType check_if_register_type(char *name){
    if(name[0] == 'r' && isdigit(name[1])){
        return TYPE_REGISTER;
    }
    return TYPE_NONE;
}

Token generate_keyword(char *current, int *current_index, int *line, int *character, Lexer lex){
    char *keyword_name = malloc(sizeof(char) * 1024);
    int keyword_length = 0;
    while(is_name(current[*current_index]) || isdigit(current[*current_index])){
        keyword_name[keyword_length] = current[*current_index];
        *current_index += 1;
        keyword_length++;
    }
    keyword_name[keyword_length] = '\0';
    TokenType type = check_if_register_type(keyword_name);
    if(type == TYPE_NONE){
        type = check_builtin_keywords(keyword_name);
    }

    if(type == TYPE_NONE){
        type = check_label_type(current, current_index);
    }

    Token token = init_token(type, keyword_name, *line, *character, lex.file_name);
    // Increment character by lenth of keyword, also subtract one because iteration occurs in lexer function as well
    *character += keyword_length - 1;
    keyword_name = realloc(keyword_name, sizeof(char) * keyword_length);
    return token;
}

Token generate_num(char *current, int *current_index, int line, int *character, Lexer lex){
    char *num_name = malloc(sizeof(char) * 16);
    int num_length = 0;

    if(current[*current_index] == '-'){
        num_name[num_length] = '-';
        *current_index += 1;
        num_length++;
    }

    while(isdigit(current[*current_index])){
        num_name[num_length] = current[*current_index];
        *current_index += 1;
        num_length++;
    }
    if(current[*current_index] != '.'){
        num_name[num_length] = '\0';
        TokenType type = TYPE_INT;
        Token token = init_token(type, num_name, line, *character, lex.file_name);
        return token;
    }
    num_name[num_length] = current[*current_index];
    *current_index += 1;
    num_length++;
    while(isdigit(current[*current_index])){
        num_name[num_length] = current[*current_index];
        *current_index += 1;
        num_length++;
    }
    num_name[num_length] = '\0';
    TokenType type = TYPE_FLOAT;
    Token token = init_token(type, num_name, line, *character, lex.file_name);

    // Increment character by lenth of number, also subtract one because iteration occurs in lexer function as well
    *character += num_length - 1;
    num_name = realloc(num_name, sizeof(char) * num_length);
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
    string_name = realloc(string_name, sizeof(char) * string_index);
    return token;
}

Lexer lexer(char *file_name){
    int length = 0;
    char *current = prepro(file_name, &length, 0);
    //printf("%s\n", current);
    //printf("%d\n", length);

    int current_index = 0;
    int line = 1;
    int character = 1;

    Lexer lex = {.stack_size = 0, .file_name = file_name};

    while(current_index < length){
        if(current[current_index] == '\n'){
            line++;
            character = 0;
        }

        if(is_name(current[current_index])){
            Token token = generate_keyword(current, &current_index, &line, &character, lex);
            push_token(&lex, token);
            current_index--;
        } else if(isdigit(current[current_index])){
            Token token = generate_num(current, &current_index, line, &character, lex);
            push_token(&lex, token);
            current_index--;
        } else if(current[current_index] == '-'){
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
            line = atoi(num.text) - 1;
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
