#include "frontend.h"

char *token_types[TT_COUNT] = {"none", "write", "exit", "builtin", "ident", 
                               ":", "(", ")", "[", "]", "{", "}", ",", ".", "=", "==", "!=", ">=", "<=", ">", "<", "+", "-", "*", "/", "%",
                               "string", "char", "integer", "float", "struct", "void", "type", "if", "else", "while", "then", 
                               "return", "end"};
    
String_View data_types[DATA_COUNT] = {
    {.data="int", .len=3},
    {.data="str", .len=3},    
    {.data="void", .len=4},        
    {.data="char", .len=4},                    
    {.data="float", .len=5},            
    {.data="ptr", .len=3},                
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
	
char get_escape(char c){
    switch(c){
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
			ASSERT(false, "unexpected escape character");
    }
}

String_View read_file_to_view(Arena *arena, char *filename) {
    FILE *file = fopen(filename, "r");
    
    fseek(file, 0, SEEK_END);
    size_t len = ftell(file);
    fseek(file, 0, SEEK_SET);    
    
    char *data = arena_alloc(arena, sizeof(char)*len);
    fread(data, sizeof(char), len, file);
    
    fclose(file);
    return (String_View){.data=data, .len=len};
}
    
bool isword(char c) {
    return isalpha(c) || isdigit(c) || c == '_';
}
	
typedef struct {
	String_View text;
	int type;
} Keyword;

Keyword keyword_list[] = {
	{LITERAL_VIEW("write"), TT_WRITE},
	{LITERAL_VIEW("exit"), TT_EXIT},		
	{LITERAL_VIEW("if"), TT_IF},	
	{LITERAL_VIEW("else"), TT_ELSE},	
	{LITERAL_VIEW("if"), TT_IF},	
	{LITERAL_VIEW("while"), TT_WHILE},	
	{LITERAL_VIEW("then"), TT_THEN},	
	{LITERAL_VIEW("return"), TT_RET},	
	{LITERAL_VIEW("end"), TT_END},
	{LITERAL_VIEW("struct"), TT_STRUCT},	
};
#define KEYWORDS_COUNT sizeof(keyword_list)/sizeof(*keyword_list)

Token_Type get_token_type(String_View view) {
	for(size_t i = 0; i < KEYWORDS_COUNT; i++) {
	    if(view_cmp(view, keyword_list[i].text)) {
	        return keyword_list[i].type;
		}
	}
    return TT_NONE;
}
	
Keyword builtins_list[] = {
	{LITERAL_VIEW("alloc"), BUILTIN_ALLOC},
	{LITERAL_VIEW("dealloc"), BUILTIN_DEALLOC},		
	{LITERAL_VIEW("store"), BUILTIN_STORE},	
	{LITERAL_VIEW("tovp"), BUILTIN_TOVP},		
	{LITERAL_VIEW("get"), BUILTIN_GET},	
};
#define BUILTIN_COUNT sizeof(builtins_list)/sizeof(*builtins_list)

Token token_get_builtin(Token token, String_View view) {
	for(size_t i = 0; i < BUILTIN_COUNT; i++) {
	    if(view_cmp(view, builtins_list[i].text)) {
			token.type = TT_BUILTIN;
			token.value.builtin = builtins_list[i].type;
			return token;
		}
	}
    return token;
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
	
Dynamic_Str generate_string(Arena *arena, String_View *view, Token token, char delim) {
    Dynamic_Str word = {0};
    *view = view_chop_left(*view);
    while(view->len > 0 && *view->data != delim) {
	    if(view->len > 1 && *view->data == '\\') {
		    //ADA_APPEND(arena, &word, *view->data);                                        
		    *view = view_chop_left(*view);
		    if(!is_valid_escape(*view->data)) {
			    PRINT_ERROR(token.loc, "unexpected escape character: `%c`", *view->data);
		    }
			ADA_APPEND(arena, &word, get_escape(*view->data));
	    } else {
		    ADA_APPEND(arena, &word, *(*view).data);
		}
	    *view = view_chop_left(*view);
    }
	ADA_APPEND(arena, &word, '\0');		
	return word;
}

Token_Arr lex(Arena *arena, Arena *string_arena, char *filename, String_View view) {
    size_t row = 1;
    Token_Arr tokens = {0};
    const char *start = view.data;
    while(view.len > 0) {
        Token token = {0};
        token.loc = (Location){.filename = filename, .row=row, .col=view.data-start};
        switch(*view.data) {
            case ':':
                token.type = TT_COLON;
                ADA_APPEND(arena, &tokens, token);                                                    
                break;
            case '(':
                token.type = TT_O_PAREN;
                ADA_APPEND(arena, &tokens, token);                                                    
                break;
            case ')':
                token.type = TT_C_PAREN;
                ADA_APPEND(arena, &tokens, token);                                                    
                break;
            case '[':
                token.type = TT_O_BRACKET;
                ADA_APPEND(arena, &tokens, token);                                                    
                break;
            case ']':
                token.type = TT_C_BRACKET;
                ADA_APPEND(arena, &tokens, token);                                                    
                break;
            case '{':
                token.type = TT_O_CURLY;
                ADA_APPEND(arena, &tokens, token);                                                    
                break;
            case '}':
                token.type = TT_C_CURLY;
                ADA_APPEND(arena, &tokens, token);                                                    
                break;
            case ',':
                token.type = TT_COMMA;
                ADA_APPEND(arena, &tokens, token);                                                    
                break;
            case '.':
                token.type = TT_DOT;
                ADA_APPEND(arena, &tokens, token);                                                    
                break;
			case '"': {
				Dynamic_Str word = generate_string(string_arena, &view, token, '"');
                if(view.len == 0 && *view.data != '"') {
                    PRINT_ERROR(token.loc, "expected closing `\"`");                            
                };
                token.type = TT_STRING;
                token.value.string = view_create(word.data, word.count);
                ADA_APPEND(arena, &tokens, token);                                    
			} break;
			case '\'': {
                Dynamic_Str word = generate_string(string_arena, &view, token, '\'');
                if(word.count > 2) {
                    PRINT_ERROR(token.loc, "character cannot be made up of multiple characters");
                }
                if(view.len == 0 && *view.data != '\'') {
                    PRINT_ERROR(token.loc, "expected closing `'` quote");                            
                };
                token.type = TT_CHAR_LIT; 
                token.value.string = view_create(word.data, word.count);
                ADA_APPEND(arena, &tokens, token);                                    
			} break;
            case '\n':
                row++;
                start = view.data;
                break;
            default: {
                if(isalpha(*view.data)) {
                    Dynamic_Str word = {0};    
                    ADA_APPEND(string_arena, &word, *view.data);                    
                    view = view_chop_left(view);
                    while(view.len > 0 && isword(*view.data)) {
                        ADA_APPEND(string_arena, &word, *view.data);            
                        view = view_chop_left(view);    
                    }
                    view.data--;
                    view.len++;
                    String_View view = view_create(word.data, word.count);
                    token.type = get_token_type(view);
                    token = handle_data_type(token, view);
                    token = token_get_builtin(token, view);
                    if(token.type == TT_NONE) {
                        token.type = TT_IDENT;
                        token.value.ident = view;
                    };
                    ADA_APPEND(arena, &tokens, token);     
                } else if(isdigit(*view.data)) {
                    Dynamic_Str num = {0};
                    token.type = TT_INT;            
                    while(view.len > 0 && (isdigit(*view.data) || *view.data == '.')) {
                        if(*view.data == '.') token.type = TT_FLOAT_LIT;
                        ADA_APPEND(arena, &num, *view.data);
                        view = view_chop_left(view);
                    }
                    view.data--;
                    view.len++;
                        
                    ADA_APPEND(arena, &num, '\0');
                    if(token.type == TT_FLOAT_LIT) token.value.floating = atof(num.data);
                    else token.value.integer = atoi(num.data);
                    ADA_APPEND(arena, &tokens, token);                        
                } else if(is_operator(view)) {
                    token = create_operator_token(filename, row, view.data-start, &view);
                    ADA_APPEND(arena, &tokens, token);                                        
				} else if(*view.data == '=') {
                    token.type = TT_EQ;
                    ADA_APPEND(arena, &tokens, token);                                        
                } else if(isspace(*view.data)) {
                    view = view_chop_left(view);                   
                    continue;
                } else {
                    PRINT_ERROR(token.loc, "unexpected token `%c`", *view.data);
                }
            }
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

Token expect_token(Token_Arr *tokens, Token_Type type) {
    Token token = token_consume(tokens);
    if(token.type != type) {
        PRINT_ERROR(token.loc, "expected type: `%s`, but found type `%s`\n", 
                    token_types[type], token_types[token.type]);
    };
	return token;
}

Node *create_node(Arena *arena, Node_Type type) {
    Node *node = arena_alloc(arena, sizeof(Node));
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
    
Builtin parse_builtin_node(Arena *arena, Token_Arr *tokens, Builtin_Type type, Nodes *structs) {
    Builtin builtin = {
        .type = type,
    };
    ADA_APPEND(arena, &builtin.value, parse_expr(arena, tokens, structs));
    while(token_peek(tokens, 0).type == TT_COMMA) {
        token_consume(tokens);
        ADA_APPEND(arena, &builtin.value, parse_expr(arena, tokens, structs));        
    }
    switch(type) {
        case BUILTIN_TOVP:
        case BUILTIN_GET:        
        case BUILTIN_ALLOC:
            builtin.return_type = TYPE_PTR;
            break;
        case BUILTIN_STORE:
        case BUILTIN_DEALLOC:
            builtin.return_type = TYPE_VOID;
            break;        
    }
    return builtin;
}

Expr *parse_primary(Arena *arena, Token_Arr *tokens, Nodes *structs) {
    Token token = token_consume(tokens);
    if(token.type != TT_INT && token.type != TT_BUILTIN && token.type != TT_FLOAT_LIT && token.type != TT_O_PAREN && token.type != TT_STRING && token.type != TT_CHAR_LIT && token.type != TT_IDENT) {
        PRINT_ERROR(token.loc, "expected int, string, char, or ident but found %s", token_types[token.type]);
    }
    Expr *expr = arena_alloc(arena, sizeof(Expr));   
    switch(token.type) {
        case TT_INT:
            *expr = (Expr){
                .type = EXPR_INT,
                .value.integer = token.value.integer,
                .loc = token.loc,
            };
            break;
        case TT_FLOAT_LIT:
            *expr = (Expr){
                .type = EXPR_FLOAT,
                .value.floating = token.value.floating,
                .loc = token.loc,
            };
            break;
        case TT_STRING:
            *expr = (Expr){
                .type = EXPR_STR,
                .value.string = token.value.string,
            };
            break;
        case TT_CHAR_LIT:
            *expr = (Expr){
                .type = EXPR_CHAR,
                .value.string = token.value.string,
            };
            break;
        case TT_BUILTIN: {
            Builtin value = parse_builtin_node(arena, tokens, token.value.builtin, structs);
            *expr = (Expr) {
                .type = EXPR_BUILTIN,
                .value.builtin = value,
                .loc = token.loc,
            };
            if(expr->value.builtin.return_type == TYPE_VOID) expr->return_type = TYPE_VOID;
        } break;
        case TT_O_PAREN:
            expr = parse_expr(arena, tokens, structs);
            if(token_consume(tokens).type != TT_C_PAREN) {
                PRINT_ERROR(token.loc, "expected `)`");   
            }
            break;
        case TT_IDENT:
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
                    Expr *arg = parse_expr(arena, tokens, structs);
                    ADA_APPEND(arena, &expr->value.func_call.args, arg);
                }
            } else if(token_peek(tokens, 0).type == TT_O_BRACKET) {
                *expr = (Expr){
                    .type = EXPR_ARR,
                    .value.array.name = token.value.ident,
                };
                token_consume(tokens); // open bracket
                expr->value.array.index = parse_expr(arena, tokens, structs);
                if(token_consume(tokens).type != TT_C_BRACKET) {
                    PRINT_ERROR(tokens->data[0].loc, "expected `]` but found `%s`\n", token_types[tokens->data[0].type]);
                }            
				if(token_peek(tokens, 0).type == TT_DOT) {
					token_consume(tokens);
					expr->type = EXPR_FIELD_ARR;
	                token = token_consume(tokens); // field name
	                expr->value.array.var_name = token.value.ident;
				}
            } else if(token_peek(tokens, 0).type == TT_DOT) {
                *expr = (Expr){
                    .type = EXPR_FIELD,
                    .value.field.structure = token.value.ident,
                };
                token_consume(tokens); // dot
                token = token_consume(tokens); // field name
                expr->value.field.var_name = token.value.ident;
            } else {
                *expr = (Expr){
                    .type = EXPR_VAR,
                    .value.variable = token.value.ident,
                };
            }
            break;
        default:
            ASSERT(false, "unexpected token");
    }
    return expr;
}

    
Expr *parse_expr_1(Arena *arena, Token_Arr *tokens, Expr *lhs, Precedence min_precedence, Nodes *structs) {
    Token lookahead = token_peek(tokens, 0);
    // make sure it's an operator
    while(op_get_prec(lookahead.type) >= min_precedence) {
        Operator op = create_operator(lookahead.type);    
        if(tokens->count > 0) {
            token_consume(tokens);
            Expr *rhs = parse_primary(arena, tokens, structs);
            lookahead = token_peek(tokens, 0);
            while(op_get_prec(lookahead.type) > op.precedence) {
                rhs = parse_expr_1(arena, tokens, rhs, op.precedence+1, structs);
                lookahead = token_peek(tokens, 0);
            }
            // allocate new lhs node to ensure old lhs does not point
            // at itself, getting stuck in a loop
            Expr *new_lhs = arena_alloc(arena, sizeof(Expr));
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
Expr *parse_expr(Arena *arena, Token_Arr *tokens, Nodes *structs) {
    return parse_expr_1(arena, tokens, parse_primary(arena, tokens, structs), 1, structs);
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

Node parse_native_node(Arena *arena, Token_Arr *tokens, int native_value, Nodes *structs) {
    Node node = {.type = TYPE_NATIVE, .loc = tokens->data[0].loc};            
    token_consume(tokens);
    Native_Call call = {0};
    Arg arg = {0};
    arg = (Arg){.type=ARG_EXPR, .value.expr=parse_expr(arena, tokens, structs)};
    expr_type_check(node.loc, arg.value.expr);
    ADA_APPEND(arena, &call.args, arg);
    call.type = native_value;
    node.value.native = call;
    return node;
}

bool is_struct(Token_Arr *tokens, Nodes *structs) {
    Token token = token_peek(tokens, 0);
    if(token.type != TT_IDENT) PRINT_ERROR(token.loc, "expected identifier but found `%s`\n", token_types[token.type]);
    for(size_t i = 0; i < structs->count; i++) {
        if(view_cmp(structs->data[i].value.structs.name, token.value.ident)) return true;
    }
    return false;
}

    
Node parse_var_dec(Arena *arena, Token_Arr *tokens, Nodes *structs) {
    Node node = {0};
    node.type = TYPE_VAR_DEC;
    node.loc = tokens->data[0].loc;
    node.value.var.name = tokens->data[0].value.ident;
	token_consume(tokens);
    expect_token(tokens, TT_COLON);
    Token name_t = token_peek(tokens, 0);
    if(name_t.type == TT_TYPE) {
        node.value.var.type = tokens->data[0].value.type;       
    } else if(is_struct(tokens, structs)) {
        node.value.var.is_struct = true;
        node.value.var.struct_name = name_t.value.ident;
    } else {
        PRINT_ERROR(token_peek(tokens, 0).loc, "expected `type` but found `%s`\n", token_types[token_peek(tokens, 0).type]);
    }
    node.value.var.is_array = token_peek(tokens, 1).type == TT_O_BRACKET || node.value.var.type == TYPE_STR;
    if(token_peek(tokens, 1).type == TT_O_BRACKET) {
        token_consume(tokens);
        token_consume(tokens);
        node.value.var.array_s = parse_expr(arena, tokens, structs);       
        expr_type_check(node.loc, node.value.var.array_s);
    }
    return node;
}

Struct get_structure(Location loc, Nodes *structs, String_View name) {
    for(size_t i = 0; i < structs->count; i++) {
        if(view_cmp(name, structs->data[i].value.structs.name)) {
            return structs->data[i].value.structs;
        }
    }
    PRINT_ERROR(loc, "unknown struct\n");
}

bool is_field(Struct *structure, String_View field) {
    for(size_t i = 0; i < structure->values.count; i++) {
        if(view_cmp(structure->values.data[i].value.var.name, field)) return true;
    }
    return false;
}
	
bool is_in_function(Blocks *blocks) {
	for(size_t i = 0; i < blocks->count; i++) {
		if(blocks->data[i].type == BLOCK_FUNC) return true;			
	}	
	return false;
}
				
int parse_reassign_left(Token_Arr *tokens, Node *node, Nodes *structs, Arena *arena) {
			Token token = token_peek(tokens, 1);
				if(token.type == TT_EQ) {
                    node->type = TYPE_VAR_REASSIGN;
                    Token name_t = token_consume(tokens);
                    token_consume(tokens); // eq
                    node->value.var.name = name_t.value.ident;                        
                } else if(token.type == TT_DOT) {
                    Token name_t = token_consume(tokens); // ident
					expect_token(tokens, TT_DOT);
                    node->type = TYPE_FIELD_REASSIGN;
                    node->value.field.structure = name_t.value.ident;
                } else if(token.type == TT_O_BRACKET) {
                    node->type = TYPE_ARR_INDEX;
                    node->value.array.name = tokens->data[0].value.ident;
                    token_consume(tokens); // ident
                    token_consume(tokens); // open bracket                        
                    node->value.array.index = parse_expr(arena, tokens, structs);
                    expr_type_check(node->loc, node->value.array.index);
					expect_token(tokens, TT_C_BRACKET);
                } else if(token.type == TT_O_PAREN) {
                    size_t i = 1;
                    while(i < tokens->count+1 && token_peek(tokens, i).type != TT_C_PAREN) i++;
                    if(i == tokens->count) {
                        PRINT_ERROR(tokens->data[0].loc, "expected `%s`\n", token_types[tokens->data[0].type]);
                    }
                    Token token = token_peek(tokens, i+1);
                    if(token.type == TT_COLON) {
                        // function dec
                        node->type = TYPE_FUNC_DEC;
                        node->value.func_dec.name = tokens->data[0].value.ident;                                        
						return i;
                    } else {
                        // function call
                        node->type = TYPE_FUNC_CALL;
                        node->value.func_call.name = tokens->data[0].value.ident;                                        
                        token_consume(tokens);                                                
                        if(token_peek(tokens, 1).type == TT_C_PAREN) {
                            // consume the open and close paren of empty funcall
                            token_consume(tokens);
                            token_consume(tokens);                    
			            } else {
					    	while(tokens->count > 0 && token_consume(tokens).type != TT_C_PAREN && i > 2) {
			                	Expr *arg = parse_expr(arena, tokens, structs);
                				ADA_APPEND(arena, &node->value.func_call.args, arg);
                  			  expr_type_check(node->loc, node->value.func_call.args.data[node->value.func_call.args.count - 1]);
			                }
			            }
			        }
				}
		return 0;
}
    
Program parse(Arena *arena, Token_Arr tokens, Blocks *block_stack) {
    // TODO: initialize the Program struct at the top of func
    Nodes root = {0};
    Functions functions = {0};
    Nodes structs = {0};
	Nodes vars = {0};
    size_t cur_label = 0;
    Size_Stack labels = {0};
    while(tokens.count > 0) {
        Node node = {.loc=tokens.data[0].loc};    
        switch(tokens.data[0].type) {
            case TT_WRITE: {
                node = parse_native_node(arena, &tokens, NATIVE_WRITE, &structs);            
                ADA_APPEND(arena, &root, node);
            } break;
            case TT_EXIT: {
                node = parse_native_node(arena, &tokens, NATIVE_EXIT, &structs);
                ADA_APPEND(arena, &root, node);
            } break;
            case TT_IDENT: {
                Token token = token_peek(&tokens, 1);
                if(token.type == TT_COLON) {
                    node = parse_var_dec(arena, &tokens, &structs);
                    token_consume(&tokens);						
                    expect_token(&tokens, TT_EQ);                
					if(block_stack->count == 0) node.value.var.global = 1;
                    if(node.value.var.is_array && node.value.var.type != TYPE_STR) {
                        if(token_consume(&tokens).type != TT_O_BRACKET) {
                            PRINT_ERROR(tokens.data[0].loc, "expected `[` but found `%s`\n", token_types[tokens.data[0].type]);
                        }
                        while(tokens.count > 0) {
                            ADA_APPEND(arena, &node.value.var.value, parse_expr(arena, &tokens, &structs));
                            expr_type_check(node.loc, node.value.var.value.data[node.value.var.value.count-1]);
                            Token next = token_consume(&tokens);
                            if(next.type == TT_COMMA) continue;
                            else if(next.type == TT_C_BRACKET) break;
                            else PRINT_ERROR(tokens.data[0].loc, "expected `,` but found `%s`\n", token_types[tokens.data[0].type]);       
                        }
                    } else if(node.value.var.is_struct) { 
						expect_token(&tokens, TT_O_CURLY);
                        Struct structure = get_structure(node.loc, &structs, node.value.var.struct_name);
                        while(tokens.count > 0 && token_peek(&tokens, 0).type != TT_C_CURLY) {
                            Token identifier = expect_token(&tokens, TT_IDENT);
                            if(!is_field(&structure, identifier.value.ident)) PRINT_ERROR(identifier.loc, "unknown field: "View_Print, View_Arg(identifier.value.ident));
                            Arg arg = {.name = identifier.value.ident, .type = ARG_EXPR};
							expect_token(&tokens, TT_EQ);
                            arg.value.expr = parse_expr(arena, &tokens, &structs);
                            Token comma_t = token_consume(&tokens);
                            ADA_APPEND(arena, &node.value.var.struct_value, arg);
                            if(comma_t.type == TT_C_CURLY) break;
                            if(comma_t.type != TT_COMMA) PRINT_ERROR(comma_t.loc, "expected `,` but found %s\n", token_types[comma_t.type]);                            
                        }
                        if(token_peek(&tokens, 0).type == TT_C_CURLY) token_consume(&tokens);
                    } else {
                        ADA_APPEND(arena, &node.value.var.value, parse_expr(arena, &tokens, &structs));    
                        expr_type_check(node.loc, node.value.var.value.data[node.value.var.value.count-1]);
                    }
					if(is_in_function(block_stack)) {
						ASSERT(functions.count > 0, "Block stack got messed up frfr");
						node.value.var.function = functions.data[functions.count-1].name;							
						ADA_APPEND(arena, &root, node);
					} else {
						ADA_APPEND(arena, &vars, node);							
					}
					break;
                } else {
					int i = parse_reassign_left(&tokens, &node, &structs, arena);
					if(node.type == TYPE_VAR_REASSIGN) {
	                    ADA_APPEND(arena, &node.value.var.value, parse_expr(arena, &tokens, &structs));
	                    expr_type_check(node.loc, node.value.var.value.data[node.value.var.value.count-1]);
	                } else if(node.type == TYPE_FIELD_REASSIGN) {
	                    expect_token(&tokens, TT_EQ);
	                    ADA_APPEND(arena, &node.value.field.value, parse_expr(arena, &tokens, &structs));
	                    expr_type_check(node.loc, node.value.field.value.data[node.value.field.value.count-1]);
	                } else if(node.type == TYPE_ARR_INDEX) {
						Token token = token_peek(&tokens, 0);
						if(token.type == TT_EQ) {
							token_consume(&tokens);
		                    ADA_APPEND(arena, &node.value.array.value, parse_expr(arena, &tokens, &structs));
		                    expr_type_check(node.loc, node.value.array.value.data[node.value.array.value.count - 1]);
						} else if(token.type == TT_DOT) {
							ASSERT(false, "unimplemented");				
						} else {
							PRINT_ERROR(node.loc, "unexpected token %s\n", token_types[token.type]);
						}
					} else if(node.type == TYPE_FUNC_DEC) {
						Function function = {0};			
						function.name = node.value.func_dec.name;								
	                    Block block = {.type = BLOCK_FUNC, .value = node.value.func_dec.name};
	                    ADA_APPEND(arena, block_stack, block);                        
	                    token_consume(&tokens);                        
	                    while(tokens.count > 0 && token_consume(&tokens).type != TT_C_PAREN && i > 2) {
	                        Node arg = parse_var_dec(arena, &tokens, &structs);
							arg.value.var.function = node.value.func_dec.name;
	                        ADA_APPEND(arena, &node.value.func_dec.args, arg);
	                        ADA_APPEND(arena, &function.args, arg);								
	                        token_consume(&tokens);
	                    }
	                    token_consume(&tokens);
	                    if(i == 2) token_consume(&tokens);
	                    node.value.func_dec.type = tokens.data[0].value.type;
						function.type = node.value.func_dec.type;
	                    token_consume(&tokens);                                                                        
	                    node.value.func_dec.label = cur_label;
						ADA_APPEND(arena, &functions, function);
	                    ADA_APPEND(arena, &labels, cur_label++);
	                } else if(node.type == TYPE_FUNC_CALL) {
	                    // function call
						if(token_peek(&tokens, 0).type == TT_DOT) {
							ASSERT(false, "unimplemented");
						}
	                } else {
	                    PRINT_ERROR(token.loc, "unexpected token `%s`\n", token_types[token.type]);
	                }
	                ADA_APPEND(arena, &root, node);                
				}
            } break;       
            case TT_STRUCT: {
                node.type = TYPE_STRUCT;        
                token_consume(&tokens);
                Token name_t = expect_token(&tokens, TT_IDENT);
                node.value.structs.name = name_t.value.ident;
				expect_token(&tokens, TT_O_CURLY);
                while(tokens.count > 0 && token_peek(&tokens, 0).type != TT_C_CURLY) {
                    Node arg = parse_var_dec(arena, &tokens, &structs);
                    ADA_APPEND(arena, &node.value.structs.values, arg);
                    token_consume(&tokens);
					expect_token(&tokens, TT_COMMA);
                }
                token_consume(&tokens);
                ADA_APPEND(arena, &structs, node);
            } break;
            case TT_RET: {
                node.type = TYPE_RET;
                token_consume(&tokens);
                node.value.expr = parse_expr(arena, &tokens, &structs);
                expr_type_check(node.loc, node.value.expr);
                ADA_APPEND(arena, &root, node);                
            } break;
            case TT_IF: {
                node.type = TYPE_IF;
                ADA_APPEND(arena, block_stack, (Block){.type=BLOCK_IF});                
                token_consume(&tokens);
                node.value.conditional = parse_expr(arena, &tokens, &structs);
                expr_type_check(node.loc, node.value.conditional);
                ADA_APPEND(arena, &root, node);
            } break;
            case TT_ELSE: {
                node.type = TYPE_ELSE;
                token_consume(&tokens);
                if(labels.count == 0 || block_stack->count == 0 || block_stack->data[block_stack->count-1].type != BLOCK_IF) PRINT_ERROR(node.loc, "`else` statement without prior `if`");
                ADA_APPEND(arena, block_stack, (Block){.type=BLOCK_ELSE});                                                						
				block_stack->count--;
                node.value.el.label1 = labels.data[--labels.count];                
                node.value.el.label2 = cur_label;
                ADA_APPEND(arena, &labels, cur_label++);
                ADA_APPEND(arena, &root, node);                
            } break;
            case TT_WHILE: {
                node.type = TYPE_WHILE;
                token_consume(&tokens);
                ADA_APPEND(arena, block_stack, (Block){.type=BLOCK_WHILE});                                
                node.value.conditional = parse_expr(arena, &tokens, &structs);
                expr_type_check(node.loc, node.value.conditional);
                ADA_APPEND(arena, &root, node);
            } break;
            case TT_THEN: {
                node.type = TYPE_THEN;
                token_consume(&tokens);                
                node.value.label.num = cur_label;
                ADA_APPEND(arena, &labels, cur_label++);
                ADA_APPEND(arena, &root, node);                
            } break;
            case TT_END: {
                node.type = TYPE_END;
                token_consume(&tokens);                                
                if(labels.count == 0 || block_stack->count == 0) PRINT_ERROR(node.loc, "`end` without an opening block");
				--block_stack->count;
                node.value.label.num = labels.data[--labels.count];   
                ADA_APPEND(arena, &root, node);                
            } break;
            case TT_STRING:
            case TT_CHAR_LIT:
            case TT_INT:            
            case TT_FLOAT_LIT:
            case TT_BUILTIN: {
                node.type = TYPE_EXPR_STMT;
                node.value.expr_stmt = parse_expr(arena, &tokens, &structs);
                ADA_APPEND(arena, &root, node);
            } break;
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
            case TT_O_CURLY:
            case TT_C_CURLY:
            case TT_COMMA:
            case TT_DOT:
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
                PRINT_ERROR(tokens.data[0].loc, "unexpected token: %s", token_types[tokens.data[0].type]);
            default:
                ASSERT(false, "invalid token detected, something went wrong in the parsing...");
        }
    }
    Program program = {0};
    program.nodes = root;
    program.functions = functions;
    program.structs = structs;
	program.vars = vars;
    return program;
}
