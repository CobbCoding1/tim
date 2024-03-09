#include "backend.h"

char *node_types[TYPE_COUNT] = {"root", "native", "expr", "var_dec", "var_reassign",
                                    "if", "else", "while", "then", "func_dec", "func_call", "return", "end"};
    
size_t data_type_s[DATA_COUNT] = {8};
    
void gen_push(Program_State *state, FILE *file, int value) {
    fprintf(file, "push %d\n", value);
    state->stack_s++;   
}
    
void gen_pop(Program_State *state, FILE *file) {
    fprintf(file, "pop\n");
    state->stack_s--;   
}

void gen_push_str(Program_State *state, FILE *file, String_View value) {
    fprintf(file, "push_str \""View_Print"\"\n", View_Arg(value));
    state->stack_s++;
}
    
void gen_indup(Program_State *state, FILE *file, size_t value) {
    fprintf(file, "indup %zu\n", value);
    state->stack_s++;
}
    
void gen_inswap(FILE *file, size_t value) {
    fprintf(file, "inswap %zu\n", value);
}
    
void gen_zjmp(Program_State *state, FILE *file, size_t label) {
    fprintf(file, "zjmp label%zu\n", label);
    state->stack_s--;
}
    
void gen_jmp(FILE *file, size_t label) {
    fprintf(file, "jmp label%zu\n", label);
}
    
void gen_while_jmp(FILE *file, size_t label) {
    fprintf(file, "jmp while%zu\n", label);
}
    
void gen_label(FILE *file, size_t label) {
    fprintf(file, "label%zu:\n", label);
}
    
void gen_func_label(FILE *file, String_View label) {
    fprintf(file, View_Print":\n", View_Arg(label));
}
    
void gen_func_call(FILE *file, String_View label) {
    fprintf(file, "call "View_Print"\n", View_Arg(label));
}
    
void gen_while_label(FILE *file, size_t label) {
    fprintf(file, "while%zu:\n", label);   
}
    
void gen_alloc(Program_State *state, FILE *file, Expr *s, size_t type_s) {
    fprintf(file, "push %zu\n", type_s);
    gen_expr(state, file, s);
    fprintf(file, "mult\n");
    fprintf(file, "alloc\n");    
}
    
void strip_off_dot(char *str) {
    while(*str != '\0' && *str != '.') {
        str++;
    }
    *str = '\0';
}

char *append_tasm_ext(char *filename) {
    size_t filename_s = strlen(filename);
    char *output = custom_realloc(NULL, sizeof(char)*filename_s);
    strncpy(output, filename, filename_s);
    strip_off_dot(output);
    char *output_filename = custom_realloc(NULL, sizeof(char)*strlen(output)+sizeof(".tasm"));
    sprintf(output_filename, "%s.tasm", output);
    return output_filename;
}
    
char *op_types[] = {"add", "sub", "mul", "div", "cmpe", "cmpne", "cmpge", "cmple", "cmpg", "cmpl"};

Function *get_func(Functions functions, String_View name) {
    for(size_t i = 0; i < functions.count; i++) {
        if(view_cmp(functions.data[i].name, name)) {
            return &functions.data[i];
        }
    }
    return false;
}

int get_variable_location(Program_State *state, String_View name) {
    for(size_t i = 0; i < state->vars.count; i++) {
        if(view_cmp(state->vars.data[i].name, name)) {
            return state->vars.data[i].stack_pos;
        }
    }
    return -1;
}

void gen_expr(Program_State *state, FILE *file, Expr *expr) {
    switch(expr->type) {
        case EXPR_BIN:
            gen_expr(state, file, expr->value.bin.lhs);
            gen_expr(state, file, expr->value.bin.rhs);
            fprintf(file, "%s\n", op_types[expr->value.bin.op.type]);                    
            state->stack_s--;
            break;
        case EXPR_INT:
            gen_push(state, file, expr->value.integer);        
            break;
        case EXPR_VAR: {
            int index = get_variable_location(state, expr->value.variable);
            if(index == -1) {
                PRINT_ERROR((Location){0}, "variable "View_Print" referenced before assignment", View_Arg(expr->value.variable));
            }
            gen_indup(state, file, state->stack_s-index); 
        } break;
        case EXPR_FUNCALL: {
            Function *function = get_func(state->functions, expr->value.func_call.name);
            if(!function) { 
                PRINT_ERROR((Location){0}, "function "View_Print" referenced before assignment\n", View_Arg(expr->value.func_call.name));
            }
            if(function->args.count != expr->value.func_call.args.count) {
                PRINT_ERROR((Location){0}, "args count do not match for function "View_Print"\n", View_Arg(function->name));
            }
            for(size_t i = 0; i < expr->value.func_call.args.count; i++) {
                gen_expr(state, file, expr->value.func_call.args.data[i]);
            }
            gen_func_call(file, expr->value.func_call.name);
        } break;
        default:
            ASSERT(false, "UNREACHABLE, %d\n", expr->type);
    }       
}

void scope_end(Program_State *state, FILE *file) {
    ASSERT(state->scope_stack.count > 0, "scope stack count == 0");
    size_t target = state->scope_stack.data[state->scope_stack.count-1];
    while(state->stack_s > target) {
        gen_pop(state, file);
    }
    while(state->vars.count > 0 && state->vars.data[state->vars.count-1].stack_pos > state->stack_s) {
        state->vars.count--;
    }
}

void ret_scope_end(Program_State *state, FILE *file) {
    ASSERT(state->ret_stack.count > 0, "scope stack count == 0");
    size_t target = state->ret_stack.data[state->ret_stack.count-1];
    if(state->stack_s > 0) state->stack_s--;
    while(state->stack_s > target) {
        gen_pop(state, file);
    }
}
    
void gen_program(Program_State *state, Nodes nodes, FILE *file) {
    for(size_t i = 0; i < nodes.count; i++) {
        Node *node = &nodes.data[i];
        switch(node->type) {
            case TYPE_NATIVE:
                switch(node->value.native.type) {
                    case NATIVE_WRITE:
                        ASSERT(node->value.native.args.count == 1, "too many arguments");
                        if(node->value.native.args.data[0].type != ARG_STRING) {
                            PRINT_ERROR(node->loc, "expected type string, but found type %s", node_types[node->value.native.args.data[0].type]);
                        };
                        fprintf(file, "; write\n");
                        gen_push_str(state, file, node->value.native.args.data[0].value.string);
                        gen_push(state, file, STDOUT);
                        fprintf(file, "native %d\n", node->value.native.type);
                        state->stack_s -= 2;
                        break;
                    case NATIVE_EXIT:
                        ASSERT(node->value.native.args.count == 1, "too many arguments");
                        if(node->value.native.args.data[0].type != ARG_EXPR) {
                            PRINT_ERROR(node->loc, "expected type int, but found type %s", node_types[node->value.native.args.data[0].type]);
                        };
                        fprintf(file, "; exit\n");                
                        fprintf(file, "; expr\n");                                
                        gen_expr(state, file, node->value.native.args.data[0].value.expr);
                        fprintf(file, "native %d\n", node->value.native.type);
                        state->stack_s--;
                        break;           
                    default:
                        ASSERT(false, "unreachable");
                }
                break;
            case TYPE_VAR_DEC: {
                fprintf(file, "; var declaration\n");                                                        
                fprintf(file, "; expr\n");                                            
                if(node->value.var.is_array) {
                    gen_alloc(state, file, node->value.var.array_s, data_type_s[node->value.var.type]);
                } else {
                    gen_expr(state, file, node->value.var.value.data[0]);
                }
                node->value.var.stack_pos = state->stack_s;                 
                DA_APPEND(&state->vars, node->value.var);    
            } break;
            case TYPE_VAR_REASSIGN: {
                fprintf(file, "; var reassign\n");                                            
                fprintf(file, "; expr\n");                                                                
                gen_expr(state, file, node->value.var.value.data[0]);
                int index = get_variable_location(state, node->value.var.name);
                if(index == -1) {
                    PRINT_ERROR((Location){0}, "variable "View_Print" referenced before assignment", View_Arg(node->value.var.name));
                }
                gen_inswap(file, state->stack_s-index);    
                gen_pop(state, file);
                DA_APPEND(&state->vars, node->value.var);    
            } break;
            case TYPE_FUNC_DEC: {
                Function function = {0};
                function.type = node->value.func_dec.type;
                function.args = node->value.func_dec.args;
                function.name = node->value.func_dec.name;
                DA_APPEND(&state->functions, function);
                DA_APPEND(&state->block_stack, BLOCK_FUNC);                
                DA_APPEND(&state->ret_stack, state->stack_s);                
                DA_APPEND(&state->scope_stack, state->stack_s);                
                for(size_t i = 0; i < function.args.count; i++) {
                    Variable var = {0};
                    var.stack_pos = ++state->stack_s;
                    var.name = function.args.data[i].value.var.name;
                    DA_APPEND(&state->vars, var);    
                }
                gen_jmp(file, node->value.func_dec.label);                                
                gen_func_label(file, function.name);
            } break;
            case TYPE_FUNC_CALL: {
                Function *function = get_func(state->functions, node->value.func_call.name);
                if(!function) { 
                    PRINT_ERROR((Location){0}, "function "View_Print" referenced before assignment\n", View_Arg(node->value.func_call.name));
                }
                if(function->args.count != node->value.func_call.args.count) {
                    PRINT_ERROR((Location){0}, "args count do not match for function "View_Print"\n", View_Arg(function->name));
                }
                for(size_t i = 0; i < node->value.func_call.args.count; i++) {
                    gen_expr(state, file, node->value.func_call.args.data[i]);
                }
                gen_func_call(file, node->value.func_call.name);
                state->stack_s -= node->value.func_call.args.count;
            } break;
            case TYPE_RET: {
                fprintf(file, "; return\n");
                size_t pos = state->scope_stack.data[state->ret_stack.count-1] + 1;
                gen_expr(state, file, node->value.expr);
                ASSERT(pos <= state->stack_s, "pos is too great");
                gen_inswap(file, state->stack_s-pos);
                size_t pre_stack_s = state->stack_s;
                ret_scope_end(state, file);
                state->stack_s = pre_stack_s;
                fprintf(file, "ret\n");
            } break;
            case TYPE_IF: {
                fprintf(file, "; if statement\n");                                                                                
                fprintf(file, "; expr\n");                                                                            
                DA_APPEND(&state->block_stack, BLOCK_IF);
                gen_expr(state, file, node->value.conditional);
            } break;
            case TYPE_ELSE: {
                fprintf(file, "; else statement\n");                                                                                
                if(state->block_stack.data[--state->block_stack.count] == BLOCK_IF) {
                    gen_jmp(file, node->value.el.label2);
                } else {
                    PRINT_ERROR((Location){0}, "expected if but found %d\n", state->block_stack.data[state->block_stack.count]);
                }
                DA_APPEND(&state->block_stack, BLOCK_ELSE);                
                gen_label(file, node->value.el.label1);
            } break;
            case TYPE_WHILE: {
                fprintf(file, "; while loop\n");                                                                                
                fprintf(file, "; expr\n");                                                                            
                DA_APPEND(&state->block_stack, BLOCK_WHILE);
                DA_APPEND(&state->while_labels, state->while_label);
                gen_while_label(file, state->while_label++);
                gen_expr(state, file, node->value.conditional);
            } break;
            case TYPE_THEN: {
                fprintf(file, "; then\n");                                                                                
                gen_zjmp(state, file, node->value.label.num);            
                DA_APPEND(&state->scope_stack, state->stack_s);
            } break;
            case TYPE_END: {
                fprintf(file, "; end\n");                                                                                                        
                if(state->block_stack.count == 0) {
                    PRINT_ERROR((Location){0}, "error: block stack: %zu\n", state->block_stack.count);
                }
                scope_end(state, file);                    
                state->scope_stack.count--;
                Block_Type block = state->block_stack.data[--state->block_stack.count];
                if(block == BLOCK_WHILE) {
                    fprintf(file, "; end while\n");                                                                                                                
                    gen_while_jmp(file, state->while_labels.data[--state->while_labels.count]);
                } else if(block == BLOCK_FUNC) {
                    fprintf(file, "; end func\n");                                                                                                                
                    fprintf(file, "ret\n");                                                                                                                                    
                }
                gen_label(file, node->value.label.num);
            } break;
            default:
                break;
        }    
    }

}
    
void generate(Program_State *state, Program *program, char *filename) {
    char *output = append_tasm_ext(filename);
    FILE *file = fopen(output, "w");
    gen_program(state, program->nodes, file);
    fprintf(file, "; exit\n");                
    gen_push(state, file, 0);
    fprintf(file, "native 60\n");
    state->stack_s--;
    //gen_program(state, program->functions, program->functions, file);        
}