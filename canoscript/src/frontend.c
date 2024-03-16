#include "frontend.h"

char *token_types[TT_COUNT] = {"none", "write", "exit", "ident", 
                               ":", "(", ")", "[", "]", ",", "=", "==", "!=", ">=", "<=", ">", "<", "+", "-", "*", "/", "%",
                               "string", "char", "integer", "float", "void", "type", "if", "else", "while", "then", "return", "end"};
    
String_View data_types[DATA_COUNT] = {
    {.data="int", .len=3},
    {.data="str", .len=3},    
    {.data="void", .len=4},        
    {.data="char", .len=4},                    
    {.data="float", .len=5},            
};    

bool is_valid_escape(char c){
    switch(c){
        case 'n':
        case 't':
        case 'v':
        case 'b':
        case 'r':
        case 'f':
        case 'a':
        case '\\':
        case '?':
        case '\'':
        case '\"':
        case '0':
            return true;
        default:
            return false;
    }
}

String_View read_file_to_view(char *filename) {
    FILE *file = fopen(filename, "r");
    
    fseek(file, 0, SEEK_END);
    size_t len = ftell(file);
    fseek(file, 0, SEEK_SET);    
    
    char *data = custom_realloc(NULL, sizeof(char)*len);
    fread(data, sizeof(char), len, file);
    
    fclose(file);
    return (String_View){.data=data, .len=len};
}
    
bool isword(char c) {
    return isalpha(c) || isdigit(c) || c == '_';
}

Token_Type get_token_type(String_View str) {
    if(view_cmp(str, view_create("write", 5))) {
        return TT_WRITE;
    } else if(view_cmp(str, view_create("exit", 4))) {
        return TT_EXIT;
    } else if(view_cmp(str, view_create("if", 2))) {
        return TT_IF;
    } else if(view_cmp(str, view_create("else", 4))) {
        return TT_ELSE;  
    } else if(view_cmp(str, view_create("while", 5))) {
        return TT_WHILE;  
    } else if(view_cmp(str, view_create("then", 4))) {
        return TT_THEN;
    } else if(view_cmp(str, view_create("return", 6))) {
        return TT_RET;
    } else if(view_cmp(str, view_create("end", 3))) {
        return TT_END;        
    }
    return TT_NONE;
}

Token handle_data_type(Token token, String_View str) {
    for(size_t i = 0; i < DATA_COUNT; i++) {
        if(view_cmp(str, data_types[i])) {
            token.type = TT_TYPE;
            token.value.type = (Type_Type)i;
        }
    }
    return token;
}
    
bool is_operator(String_View view) {
    switch(*view.data) {
        case '+':
        case '-':
        case '*':
        case '/':
        case '>':
        case '<':
        case '%':
            return true;        
        case '=':
            if(view.len > 1 && view.data[1] == '=') return true;        
            return false;
        case '!':
            if(view.len > 1 && view.data[1] == '=') return true;        
            return false;
        default:
            return false;
    }
}
    
Token create_operator_token(char *filename, size_t row, size_t col, String_View *view) {
    Token token = {0};
    token.loc = (Location){
        .filename = filename,
        .row = row,
        .col = col,
    };
    switch(*view->data) {
        case '+':
            token.type = TT_PLUS;
            break;
        case '-':
            token.type = TT_MINUS;
            break;
        case '*':
            token.type = TT_MULT;
            break;
        case '/':
            token.type = TT_DIV;
            break;
        case '%':
            token.type = TT_MOD;
            break;
        case '=':
            if(view->len > 1 && view->data[1] == '=') {
                *view = view_chop_left(*view);
                token.type = TT_DOUBLE_EQ;        
            }
            break;
        case '!':
            if(view->len > 1 && view->data[1] == '=') {
                *view = view_chop_left(*view);
                token.type = TT_NOT_EQ;        
            }
            break;
        case '>':
            if(view->len > 1 && view->data[1] == '=') {
                *view = view_chop_left(*view);
                token.type = TT_GREATER_EQ;        
            } else {
                *view = view_chop_left(*view);
                token.type = TT_GREATER;        
            }
            break;
        case '<':
            if(view->len > 1 && view->data[1] == '=') {
                *view = view_chop_left(*view);
                token.type = TT_LESS_EQ;        
            } else {
                *view = view_chop_left(*view);
                token.type = TT_LESS;
            }
            break;
        default:
            ASSERT(false, "unreachable");
    }
    return token;
}
    
void print_token_arr(Token_Arr arr) {
    for(size_t i = 0; i < arr.count; i++) {
        printf("%zu:%zu: %s, "View_Print"\n", arr.data[i].loc.row, arr.data[i].loc.col, token_types[arr.data[i].type], View_Arg(arr.data[i].value.string));
    }
}

Token_Arr lex(char *filename, String_View view) {
    size_t row = 1;
    Token_Arr tokens = {0};
    char *start = view.data;
    while(view.len > 0) {
        Token token = {0};
        token.loc = (Location){.filename = filename, .row=row, .col=view.data-start};
        if(isalpha(*view.data)) {
            Dynamic_Str word = {0};    
            DA_APPEND(&word, *view.data);                    
            view = view_chop_left(view);
            while(view.len > 0 && isword(*view.data)) {
                DA_APPEND(&word, *view.data);            
                view = view_chop_left(view);    
            }
            view.data--;
            view.len++;
            token.type = get_token_type(view_create(word.data, word.count));
            token = handle_data_type(token, view_create(word.data, word.count));
            if(token.type == TT_NONE) {
                token.type = TT_IDENT;
                char *ident = custom_realloc(NULL, sizeof(char)*word.count);
                strncpy(ident, word.data, word.count);
                token.value.ident = view_create(ident, word.count);
            };
            DA_APPEND(&tokens, token);     
            free(word.data);
        } else if(*view.data == '"') {
            Dynamic_Str word = {0};
            view = view_chop_left(view);
            while(view.len > 0 && *view.data != '"') {
                if(view.len > 1 && *view.data == '\\') {
                    DA_APPEND(&word, *view.data);                                        
                    view = view_chop_left(view);
                    if(!is_valid_escape(*view.data)) {
                        PRINT_ERROR(token.loc, "unexpected escape character: `%c`", *view.data);
                    }
                }
                DA_APPEND(&word, *view.data);
                view = view_chop_left(view);
            }
            if(view.len == 0 && *view.data != '"') {
                PRINT_ERROR(token.loc, "expected closing `\"`");                            
            };
            token.type = TT_STRING;
            token.value.string = view_create(word.data, word.count);
            DA_APPEND(&tokens, token);                                    
        } else if(*view.data == '\'') {
            Dynamic_Str word = {0};
            view = view_chop_left(view);
            while(view.len > 0 && *view.data != '\'') {
                if(view.len > 1 && *view.data == '\\') {
                    DA_APPEND(&word, *view.data);                                        
                    view = view_chop_left(view);
                    if(!is_valid_escape(*view.data)) {
                        PRINT_ERROR(token.loc, "unexpected escape character: `%c`", *view.data);
                    }
                }
                DA_APPEND(&word, *view.data);
                view = view_chop_left(view);
            }
            if(word.count > 2) {
                PRINT_ERROR(token.loc, "character cannot be made up of multiple characters");
            }
            if(view.len == 0 && *view.data != '\'') {
                PRINT_ERROR(token.loc, "expected closing `'` quote");                            
            };
            token.type = TT_CHAR_LIT; 
            token.value.string = view_create(word.data, word.count);
            DA_APPEND(&tokens, token);                                    
        } else if(isdigit(*view.data)) {
            Dynamic_Str num = {0};
            token.type = TT_INT;            
            while(view.len > 0 && (isdigit(*view.data) || *view.data == '.')) {
                if(*view.data == '.') token.type = TT_FLOAT_LIT;
                DA_APPEND(&num, *view.data);
                view = view_chop_left(view);
            }
            view.data--;
            view.len++;
                
            DA_APPEND(&num, '\0');
            if(token.type == TT_FLOAT_LIT) token.value.floating = atof(num.data);
            else token.value.integer = atoi(num.data);
            DA_APPEND(&tokens, token);                        
        } else if(is_operator(view)) {
            token = create_operator_token(filename, row, view.data-start, &view);
            DA_APPEND(&tokens, token);                                        
        } else if(*view.data == ':') {
            token.type = TT_COLON;
            DA_APPEND(&tokens, token);                                                    
        } else if(*view.data == '=') {
            token.type = TT_EQ;
            DA_APPEND(&tokens, token);                                                    
        } else if(*view.data == '(') {
            token.type = TT_O_PAREN;
            DA_APPEND(&tokens, token);                                                    
        } else if(*view.data == ')') {
            token.type = TT_C_PAREN;                
            DA_APPEND(&tokens, token);                                                    
        } else if(*view.data == '[') {
            token.type = TT_O_BRACKET;
            DA_APPEND(&tokens, token);                                                    
        } else if(*view.data == ']') {
            token.type = TT_C_BRACKET;            
            DA_APPEND(&tokens, token);                                                    
        } else if(*view.data == ',') {
            token.type = TT_COMMA;
            DA_APPEND(&tokens, token);                                                    
        } else if(*view.data == '\n') {
            row++;
            start = view.data;
        }
        view = view_chop_left(view);
    }
    return tokens;
}
    
Token token_consume(Token_Arr *tokens) {
    ASSERT(tokens->count != 0, "out of tokens");
    tokens->count--;
    return *tokens->data++;
}

Token token_peek(Token_Arr *tokens, size_t peek_by) {
    if(tokens->count <= peek_by) {
        return (Token){0};
    }
    return tokens->data[peek_by];
}

void expect_token(Token_Arr *tokens, Token_Type type) {
    token_consume(tokens);
    if(tokens->data[0].type != type) {
        PRINT_ERROR(tokens->data[0].loc, "expected type: `%s`, but found type `%s`\n", 
                    token_types[type], token_types[tokens->data[0].type]);
    };
}

Node *create_node(Node_Type type) {
    Node *node = custom_realloc(NULL, sizeof(Node));
    node->type = type;
    return node;
}
    
Precedence op_get_prec(Token_Type type) {
    switch(type) {
        case TT_PLUS:
        case TT_MINUS:
        case TT_DOUBLE_EQ:
        case TT_NOT_EQ:
        case TT_GREATER_EQ:
        case TT_LESS_EQ:
        case TT_GREATER:
        case TT_LESS:
            return PREC_1;
        case TT_MULT:
        case TT_DIV:
        case TT_MOD:
            return PREC_2;
            break;
        default:
            return PREC_0;
    }
}

Operator create_operator(Token_Type type) {
    Operator op = {0};
    op.precedence = op_get_prec(type);
    switch(type) {
        case TT_PLUS:
            op.type = OP_PLUS;
            break;        
        case TT_MINUS:
            op.type = OP_MINUS;
            break;        
        case TT_MULT:
            op.type = OP_MULT;
            break;        
        case TT_DIV:
            op.type = OP_DIV;
            break;
        case TT_MOD:
            op.type = OP_MOD;
            break;
        case TT_DOUBLE_EQ:
            op.type = OP_EQ;
            break;
        case TT_NOT_EQ:
            op.type = OP_NOT_EQ;
            break;
        case TT_GREATER_EQ:
            op.type = OP_GREATER_EQ;
            break;
        case TT_LESS_EQ:
            op.type = OP_LESS_EQ;
            break;
        case TT_GREATER:
            op.type = OP_GREATER;
            break;
        case TT_LESS:
            op.type = OP_LESS;
            break;
        default:
            return (Operator){0};
    }
    return op;
}

bool token_is_op(Token token) {
    switch(token.type) {
        case TT_PLUS:
        case TT_MINUS:
        case TT_MULT:
        case TT_DIV:
            return true;    
        default:
            return false;
    }
}

void print_expr(Expr *expr) {
    if(expr->type == EXPR_INT) {
        printf("int: %d\n", expr->value.integer);
    } else {
        print_expr(expr->value.bin.lhs);
        print_expr(expr->value.bin.rhs);        
    }
}

Expr *parse_expr(Token_Arr *tokens);
    
// parse primary takes a primary, currently only supports integers
// but in the future, could be identifiers, etc
Expr *parse_primary(Token_Arr *tokens) {
    Token token = token_consume(tokens);
    if(token.type != TT_INT && token.type != TT_FLOAT_LIT && token.type != TT_O_PAREN && token.type != TT_STRING && token.type != TT_CHAR_LIT && token.type != TT_IDENT) {
        PRINT_ERROR(token.loc, "expected int, string, char, or ident but found %s", token_types[token.type]);
    }
    Expr *expr = custom_realloc(NULL, sizeof(Expr));   
    if(token.type == TT_INT) {
        *expr = (Expr){
            .type = EXPR_INT,
            .value.integer = token.value.integer,
            .loc = token.loc,
        };
    } else if(token.type == TT_FLOAT_LIT) {
        *expr = (Expr){
            .type = EXPR_FLOAT,
            .value.floating = token.value.floating,
            .loc = token.loc,
        };
    } else if(token.type == TT_STRING) {
        *expr = (Expr){
            .type = EXPR_STR,
            .value.string = token.value.string,
        };
    } else if(token.type == TT_CHAR_LIT) {
        *expr = (Expr){
            .type = EXPR_CHAR,
            .value.string = token.value.string,
        };
    } else if(token.type == TT_O_PAREN) {
        Expr *expr = parse_expr(tokens);
        if(token_consume(tokens).type != TT_C_PAREN) {
            PRINT_ERROR(token.loc, "expected `)`");   
        }
        return expr;
    } else if(token.type == TT_IDENT) {
        if(token_peek(tokens, 0).type == TT_O_PAREN) {
            *expr = (Expr){
                .type = EXPR_FUNCALL,
                .value.func_call.name = token.value.ident,
            };
            if(token_peek(tokens, 1).type == TT_C_PAREN) {
                token_consume(tokens);
                token_consume(tokens);                
                return expr;
            }
            while(tokens->count > 0 && token_consume(tokens).type != TT_C_PAREN) {
                Expr *arg = parse_expr(tokens);
                DA_APPEND(&expr->value.func_call.args, arg);
            }
        } else if(token_peek(tokens, 0).type == TT_O_BRACKET) {
            *expr = (Expr){
                .type = EXPR_ARR,
                .value.array.name = token.value.ident,
            };
            token_consume(tokens); // open bracket
            expr->value.array.index = parse_expr(tokens);
            if(token_consume(tokens).type != TT_C_BRACKET) {
                PRINT_ERROR(tokens->data[0].loc, "expected `]` but found `%s`\n", token_types[tokens->data[0].type]);
            }            
        } else {
            *expr = (Expr){
                .type = EXPR_VAR,
                .value.variable = token.value.ident,
            };
        }
    }
    return expr;
}

    
Expr *parse_expr_1(Token_Arr *tokens, Expr *lhs, Precedence min_precedence) {
    Token lookahead = token_peek(tokens, 0);
    // make sure it's an operator
    while(op_get_prec(lookahead.type) >= min_precedence) {
        Operator op = create_operator(lookahead.type);    
        if(tokens->count > 0) {
            token_consume(tokens);
            Expr *rhs = parse_primary(tokens);
            lookahead = token_peek(tokens, 0);
            while(op_get_prec(lookahead.type) > op.precedence) {
                rhs = parse_expr_1(tokens, rhs, op.precedence+1);
                lookahead = token_peek(tokens, 0);
            }
            // allocate new lhs node to ensure old lhs does not point
            // at itself, getting stuck in a loop
            Expr *new_lhs = custom_realloc(NULL, sizeof(Expr));
            *new_lhs = (Expr) {
                .type = EXPR_BIN,
                .value.bin.lhs = lhs,
                .value.bin.rhs = rhs,
                .value.bin.op = op,
            };
            lhs = new_lhs;
        }
    }
    return lhs;
}
    
Expr *parse_expr(Token_Arr *tokens) {
    return parse_expr_1(tokens, parse_primary(tokens), 1);
}

char *expr_types[EXPR_COUNT] = {"bin", "int", "str", "char", "var", "func", "arr"};

Expr_Type expr_type_check(Location loc, Expr *expr) {
    return expr->type;
    if(expr->type != EXPR_BIN) return expr->type;
    Expr_Type typel = expr_type_check(loc, expr->value.bin.lhs);
    Expr_Type typer = expr_type_check(loc, expr->value.bin.rhs);
    if(typel != typer && typel != EXPR_VAR && typer != EXPR_VAR && 
       typel != EXPR_ARR && typer != EXPR_ARR && typel != EXPR_FUNCALL && typer != EXPR_FUNCALL) {
        PRINT_ERROR(loc, "expected type `%s` but found type `%s`", expr_types[typel], expr_types[typer]);
    }
    return typel;
}

Node parse_native_node(Token_Arr *tokens, Token_Type type, int native_value) {
    Node node = {.type = TYPE_NATIVE, .loc = tokens->data[0].loc};            
    token_consume(tokens);
    Token token = tokens->data[0];
    Native_Call call = {0};
    Arg arg = {0};
    switch(type) {
        case TT_STRING:    
        case TT_INT: {
            if(token.type != TT_INT && token.type != TT_STRING && token.type != TT_CHAR_LIT && token.type != TT_IDENT) {
                PRINT_ERROR(token.loc, "expected int, string, char, or ident but found %s\n", token_types[token.type]);
            }
            arg = (Arg){.type=ARG_EXPR, .value.expr=parse_expr(tokens)};
            expr_type_check(node.loc, arg.value.expr);
        } break;
        default:
            PRINT_ERROR(tokens->data[0].loc, "unexpected type: %s\n", token_types[type]);
    }
    DA_APPEND(&call.args, arg);
    call.type = native_value;
    node.value.native = call;
    return node;
}
    
Node parse_var_dec(Token_Arr *tokens) {
    Node node = {0};
    node.type = TYPE_VAR_DEC;
    node.loc = tokens->data[0].loc;
    node.value.var.name = tokens->data[0].value.ident;
    expect_token(tokens, TT_COLON);
    expect_token(tokens, TT_TYPE);                
    node.value.var.type = tokens->data[0].value.type;
    node.value.var.is_array = token_peek(tokens, 1).type == TT_O_BRACKET || node.value.var.type == TYPE_STR;
    if(token_peek(tokens, 1).type == TT_O_BRACKET) {
        token_consume(tokens);
        token_consume(tokens);
        node.value.var.array_s = parse_expr(tokens);       
        expr_type_check(node.loc, node.value.var.array_s);
    }
    return node;
}
    
Program parse(Token_Arr tokens, Blocks *block_stack) {
    Nodes root = {0};
    Nodes functions = {0};
    size_t cur_label = 0;
    Size_Stack labels = {0};
    while(tokens.count > 0) {
        Node node = {.loc=tokens.data[0].loc};    
        switch(tokens.data[0].type) {
            case TT_WRITE: {
                node = parse_native_node(&tokens, TT_STRING, NATIVE_WRITE);            
                DA_APPEND(&root, node);
            } break;
            case TT_EXIT: {
                node = parse_native_node(&tokens, TT_INT, NATIVE_EXIT);
                DA_APPEND(&root, node);
            } break;
            case TT_IDENT: {
                Token token = token_peek(&tokens, 1);
                if(token.type == TT_COLON) {
                    node = parse_var_dec(&tokens);
                    expect_token(&tokens, TT_EQ);                
                    token_consume(&tokens);
                    if(node.value.var.is_array && node.value.var.type != TYPE_STR) {
                        if(token_consume(&tokens).type != TT_O_BRACKET) {
                            PRINT_ERROR(tokens.data[0].loc, "expected `[` but found `%s`\n", token_types[tokens.data[0].type]);
                        }
                        while(tokens.count > 0) {
                            DA_APPEND(&node.value.var.value, parse_expr(&tokens));
                            expr_type_check(node.loc, node.value.var.value.data[node.value.var.value.count-1]);
                            Token next = token_consume(&tokens);
                            if(next.type == TT_COMMA) continue;
                            else if(next.type == TT_C_BRACKET) break;
                            else PRINT_ERROR(tokens.data[0].loc, "expected `,` but found `%s`\n", token_types[tokens.data[0].type]);       
                        }
                    } else {
                        DA_APPEND(&node.value.var.value, parse_expr(&tokens));    
                        expr_type_check(node.loc, node.value.var.value.data[node.value.var.value.count-1]);
                    }
                } else if(token.type == TT_EQ) {
                    node.type = TYPE_VAR_REASSIGN;
                    node.value.var.name = tokens.data[0].value.ident;
                        
                    token_consume(&tokens); // ident
                    token_consume(&tokens); // equal                   
                    
                    DA_APPEND(&node.value.var.value, parse_expr(&tokens));
                    expr_type_check(node.loc, node.value.var.value.data[node.value.var.value.count-1]);
                } else if(token.type == TT_O_BRACKET) {
                    node.type = TYPE_ARR_INDEX;
                    node.value.array.name = tokens.data[0].value.ident;
                    token_consume(&tokens); // ident
                    token_consume(&tokens); // open bracket                        
                    node.value.array.index = parse_expr(&tokens);
                    expr_type_check(node.loc, node.value.array.index);
                    if(token_consume(&tokens).type != TT_C_BRACKET) {
                        PRINT_ERROR(tokens.data[0].loc, "expected `]` but found `%s`\n", token_types[tokens.data[0].type]);
                    }                 
                    if(token_consume(&tokens).type != TT_EQ) {
                        PRINT_ERROR(tokens.data[0].loc, "expected `=` but found `%s`\n", token_types[tokens.data[0].type]);
                    }                 
                    DA_APPEND(&node.value.array.value, parse_expr(&tokens));
                    expr_type_check(node.loc, node.value.array.value.data[node.value.array.value.count - 1]);
                } else if(token.type == TT_O_PAREN) {
                    size_t i = 1;
                    while(i < tokens.count+1 && token_peek(&tokens, i).type != TT_C_PAREN) i++;
                    if(i == tokens.count) {
                        PRINT_ERROR(tokens.data[0].loc, "expected `%s`\n", token_types[tokens.data[0].type]);
                    }
                    Token token = token_peek(&tokens, i+1);
                    if(token.type == TT_COLON) {
                        // function dec
                        node.type = TYPE_FUNC_DEC;
                        node.value.func_dec.name = tokens.data[0].value.ident;                                        
                        Block block = {.type = BLOCK_FUNC, .value = node.value.func_dec.name};
                        DA_APPEND(block_stack, block);                        
                        token_consume(&tokens);                        
                        while(tokens.count > 0 && token_consume(&tokens).type != TT_C_PAREN && i > 2) {
                            Node arg = parse_var_dec(&tokens);
                            DA_APPEND(&node.value.func_dec.args, arg);
                            token_consume(&tokens);
                        }
                        token_consume(&tokens);
                        if(i == 2) token_consume(&tokens);
                        node.value.func_dec.type = tokens.data[0].value.type;
                        token_consume(&tokens);                                                                        
                        node.value.func_dec.label = cur_label;
                        DA_APPEND(&labels, cur_label++);
                    } else {
                        // function call
                        node.type = TYPE_FUNC_CALL;
                        node.value.func_call.name = tokens.data[0].value.ident;                                        
                        token_consume(&tokens);                                                
                        while(tokens.count > 0 && token_consume(&tokens).type != TT_C_PAREN && i > 2) {
                            Expr *arg = parse_expr(&tokens);
                            DA_APPEND(&node.value.func_call.args, arg);
                            expr_type_check(node.loc, node.value.func_call.args.data[node.value.func_call.args.count - 1]);
                        }
                    }
                } else {
                    PRINT_ERROR(token.loc, "unexpected token `%s`\n", token_types[token.type]);
                }
                DA_APPEND(&root, node);                
            } break;       
            case TT_RET: {
                node.type = TYPE_RET;
                token_consume(&tokens);
                node.value.expr = parse_expr(&tokens);
                expr_type_check(node.loc, node.value.expr);
                DA_APPEND(&root, node);                
            } break;
            case TT_IF: {
                node.type = TYPE_IF;
                DA_APPEND(block_stack, (Block){.type=BLOCK_IF});                
                token_consume(&tokens);
                node.value.conditional = parse_expr(&tokens);
                expr_type_check(node.loc, node.value.conditional);
                DA_APPEND(&root, node);
            } break;
            case TT_ELSE: {
                node.type = TYPE_ELSE;
                token_consume(&tokens);
                DA_APPEND(block_stack, (Block){.type=BLOCK_ELSE});                                                
                if(labels.count == 0) PRINT_ERROR(node.loc, "`else` statement without prior `if`");
                node.value.el.label1 = labels.data[--labels.count];                
                node.value.el.label2 = cur_label;
                DA_APPEND(&labels, cur_label++);
                DA_APPEND(&root, node);                
            } break;
            case TT_WHILE: {
                node.type = TYPE_WHILE;
                token_consume(&tokens);
                DA_APPEND(block_stack, (Block){.type=BLOCK_WHILE});                                
                node.value.conditional = parse_expr(&tokens);
                expr_type_check(node.loc, node.value.conditional);
                DA_APPEND(&root, node);
            } break;
            case TT_THEN: {
                node.type = TYPE_THEN;
                token_consume(&tokens);                
                node.value.label.num = cur_label;
                DA_APPEND(&labels, cur_label++);
                DA_APPEND(&root, node);                
            } break;
            case TT_END: {
                node.type = TYPE_END;
                token_consume(&tokens);                                
                if(labels.count == 0) PRINT_ERROR(node.loc, "`end` without an opening block");
                node.value.label.num = labels.data[--labels.count];   
                DA_APPEND(&root, node);                
            } break;
            case TT_STRING:
            case TT_CHAR_LIT:
            case TT_INT:            
            case TT_FLOAT_LIT:
            case TT_VOID:
            case TT_PLUS:
            case TT_COLON:
            case TT_EQ:
            case TT_DOUBLE_EQ:
            case TT_NOT_EQ:
            case TT_O_PAREN:
            case TT_C_PAREN:
            case TT_O_BRACKET:
            case TT_C_BRACKET:
            case TT_COMMA:
            case TT_GREATER_EQ:
            case TT_LESS_EQ:
            case TT_GREATER:
            case TT_LESS:
            case TT_TYPE:
            case TT_MINUS:
            case TT_MULT:
            case TT_DIV:
            case TT_MOD:
            case TT_NONE:
            case TT_COUNT:
                printf("type: %s %d\n", token_types[tokens.data[0].type], tokens.data[0].value.integer);
                ASSERT(false, "unreachable");
        }
    }
    Program program = {0};
    program.nodes = root;
    program.functions = functions;
    return program;
}
