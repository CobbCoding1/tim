#include "backend.h"

char *node_types[TYPE_COUNT] = {"root", "native", "expr", "var_dec", "var_reassign",
                                "if", "else", "while", "then", "func_dec", "func_call", "return", "end"};
    
size_t data_type_s[DATA_COUNT] = {8, 1, 1, 1, 8, 8};
    
// TODO: add ASSERTs to all "popping" functions
void gen_push(Program_State *state, FILE *file, int value) {
    fprintf(file, "push %d\n", value);
    state->stack_s++;   
}
    
void gen_push_float(Program_State *state, FILE *file, double value) {
    fprintf(file, "push %f\n", value);
    state->stack_s++;   
}
    
void gen_push_char(Program_State *state, FILE *file, String_View value) {
    fprintf(file, "push '"View_Print"'\n", View_Arg(value));
    state->stack_s++;   
}
    
void gen_pop(Program_State *state, FILE *file) {
    fprintf(file, "pop\n");
    state->stack_s--;   
}
    
void gen_add(Program_State *state, FILE *file) {
    fprintf(file, "add\n");
    state->stack_s--;   
}
    
void gen_sub(Program_State *state, FILE *file) {
    fprintf(file, "sub\n");
    state->stack_s--;   
}

void gen_mul(Program_State *state, FILE *file) {
    fprintf(file, "mul\n");
    state->stack_s--;   
}

void gen_div(Program_State *state, FILE *file) {
    fprintf(file, "div\n");
    state->stack_s--;   
}

void gen_push_str(Program_State *state, FILE *file, String_View value) {
    fprintf(file, "push_str \""View_Print"\"\n", View_Arg(value));
    state->stack_s += 1;
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
    gen_push(state, file, type_s);
    gen_expr(state, file, s);
    gen_mul(state, file);
    fprintf(file, "alloc\n");    
}
    
void gen_struct_alloc(Program_State *state, FILE *file, size_t total_s) {
    gen_push(state, file, total_s);
    fprintf(file, "alloc\n");    
}

void gen_dup(Program_State *state, FILE *file) {
    fprintf(file, "dup\n");
    state->stack_s++;
}

void gen_offset(Program_State *state, FILE *file, size_t offset) {
    gen_dup(state, file);
    gen_push(state, file, offset);
    gen_add(state, file);
    fprintf(file, "tovp\n");
}

void gen_write(Program_State *state, FILE *file) {
    fprintf(file, "write\n");    
    state->stack_s -= 3;
}
    
void gen_read(Program_State *state, FILE *file) {
    fprintf(file, "read\n");    
    state->stack_s -= 1;
}
    
void gen_arr_offset(Program_State *state, FILE *file, size_t var_index, Expr *arr_index, Type_Type type) {
    // i dont remember why this was here
    //ASSERT(var_index > 1, "stack was corrupted");
    fprintf(file, "; offset\n");                                                                                
    gen_indup(state, file, state->stack_s-var_index);    
    gen_expr(state, file, arr_index);
    gen_push(state, file, data_type_s[type]);            
    gen_mul(state, file);
    gen_add(state, file);
    fprintf(file, "tovp\n");            
}

void gen_struct_offset(Program_State *state, FILE *file, Type_Type type, size_t var_index, size_t offset) {
    fprintf(file, "; offset\n");                                                                                
 //   gen_indup(state, file, state->stack_s-var_index);    
    gen_dup(state, file);
    gen_push(state, file, offset);
    gen_push(state, file, data_type_s[type]);            
    gen_mul(state, file);
    gen_add(state, file);
    fprintf(file, "tovp\n");            
}
    
void gen_struct_value(Program_State *state, FILE *file, size_t field_pos, Node *field, Node *value) {
    for(size_t i = 0; i < value->value.var.struct_value.count; i++) {
        Arg var = value->value.var.struct_value.data[i];
        if(view_cmp(field->value.var.name, var.name)) {
            gen_struct_offset(state, file, field->value.var.type, value->value.var.stack_pos, field_pos);
            gen_expr(state, file, var.value.expr);
            gen_push(state, file, data_type_s[field->value.var.type]);
            gen_write(state, file);
        }
    }
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
    
char *op_types[] = {"add", "sub", "mul", "div", "mod", "cmpe", "cmpne", "cmpge", "cmple", "cmpg", "cmpl"};

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
    
Type_Type get_variable_type(Program_State *state, String_View name) {
    for(size_t i = 0; i < state->vars.count; i++) {
        if(view_cmp(state->vars.data[i].name, name)) {
            return state->vars.data[i].type;
        }
    }
    return -1;
}
    
void gen_builtin(Program_State *state, FILE *file, Expr *expr) {
    ASSERT(expr->type == EXPR_BUILTIN, "type is incorrect");
    switch(expr->value.builtin.type) {
        case BUILTIN_ALLOC: {
            gen_expr(state, file, expr->value.builtin.value.data[0]);               
            fprintf(file, "alloc\n");
        } break;
        case BUILTIN_DEALLOC: {
            gen_expr(state, file, expr->value.builtin.value.data[0]);               
            fprintf(file, "dealloc\n");
            state->stack_s--;
        } break;
        case BUILTIN_TOVP: {
            gen_expr(state, file, expr->value.builtin.value.data[0]);               
            fprintf(file, "tovp\n");
        } break;
        case BUILTIN_STORE: {
            if(expr->value.builtin.value.count != 3) {
                PRINT_ERROR(expr->loc, "incorrect arg amounts for store");
            }
            for(size_t i = 0; i < expr->value.builtin.value.count; i++) {
                gen_expr(state, file, expr->value.builtin.value.data[i]);
            }
            fprintf(file, "write\n");
            state->stack_s -= 3;
        } break;
        case BUILTIN_GET: {
            if(expr->value.builtin.value.count != 2) {
                PRINT_ERROR(expr->loc, "incorrect arg amounts for get");
            }
            for(size_t i = 0; i < expr->value.builtin.value.count; i++) {
                gen_expr(state, file, expr->value.builtin.value.data[i]);
            }
            fprintf(file, "read\n");
            state->stack_s -= 1;
        } break;
    }
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
        case EXPR_FLOAT:
            gen_push_float(state, file, expr->value.floating);        
            break;
        case EXPR_STR:
            gen_push_str(state, file, expr->value.string);
            break;
        case EXPR_CHAR:
            gen_push_char(state, file, expr->value.string);
            break;
        case EXPR_VAR: {
            int index = get_variable_location(state, expr->value.variable);
            if(index == -1) {
                PRINT_ERROR(expr->loc, "variable `"View_Print"` referenced before assignment", View_Arg(expr->value.variable));
            }
            gen_indup(state, file, state->stack_s-index); 
        } break;
        case EXPR_FUNCALL: {
            Function *function = get_func(state->functions, expr->value.func_call.name);
            if(!function) { 
                PRINT_ERROR(expr->loc, "function `"View_Print"` referenced before assignment\n", View_Arg(expr->value.func_call.name));
            }
            if(function->args.count != expr->value.func_call.args.count) {
                PRINT_ERROR(expr->loc, "args count do not match for function `"View_Print"`\n", View_Arg(function->name));
            }
            for(size_t i = 0; i < expr->value.func_call.args.count; i++) {
                gen_expr(state, file, expr->value.func_call.args.data[i]);
            }
            gen_func_call(file, expr->value.func_call.name);
        } break;
        case EXPR_ARR: {
            int index = get_variable_location(state, expr->value.array.name);
            if(index == -1) {
                PRINT_ERROR(expr->loc, "variable `"View_Print"` referenced before assignment", View_Arg(expr->value.array.name));
            }
            Type_Type type = get_variable_type(state, expr->value.array.name);                        
            gen_arr_offset(state, file, index, expr->value.array.index, type);
            gen_push(state, file, data_type_s[type]);
            gen_read(state, file);
        } break;
        case EXPR_BUILTIN: {
            gen_builtin(state, file, expr);   
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
    
Node get_struct(Nodes structs, String_View name) {
    for(size_t i = 0; i < structs.count; i++) {
        if(view_cmp(name, structs.data[i].value.structs.name)) {
            return structs.data[i];
        }
    }
    PRINT_ERROR((Location){0}, "unknown struct\n");
}
    
void gen_program(Program_State *state, Nodes nodes, Nodes structs, FILE *file) {
    for(size_t i = 0; i < nodes.count; i++) {
        Node *node = &nodes.data[i];
        switch(node->type) {
            case TYPE_NATIVE:
                switch(node->value.native.type) {
                    case NATIVE_WRITE: {
                        if(node->value.native.args.count > 1) {
                            fprintf(stderr, "error: too many args\n");
                            exit(1);
                        }
                        fprintf(file, "; write\n");
                        gen_expr(state, file, node->value.native.args.data[0].value.expr);
                        gen_push(state, file, STDOUT);
                        fprintf(file, "native %d\n", node->value.native.type);
                        state->stack_s -= 2;
                    } break;
                    case NATIVE_EXIT: {
                        ASSERT(node->value.native.args.count == 1, "too many arguments");
                        if(node->value.native.args.data[0].type != ARG_EXPR) {
                            PRINT_ERROR(node->loc, "expected type int, but found type %s", node_types[node->value.native.args.data[0].type]);
                        };
                        fprintf(file, "; exit\n");                
                        fprintf(file, "; expr\n");                                
                        gen_expr(state, file, node->value.native.args.data[0].value.expr);
                        fprintf(file, "native %d\n", node->value.native.type);
                        state->stack_s--;
                    } break;           
                    default:
                        ASSERT(false, "unreachable");
                }
                break;
            case TYPE_VAR_DEC: {
                fprintf(file, "; var declaration\n");                                                        
                if(node->value.var.is_array && node->value.var.type != TYPE_STR) {
                    fprintf(file, "; array allocation\n"); 
                    gen_alloc(state, file, node->value.var.array_s, data_type_s[node->value.var.type]);
                    for(size_t i = 0; i < node->value.var.value.count; i++) {
                        fprintf(file, "; index %zu expr\n", i);                         
                        gen_offset(state, file, data_type_s[node->value.var.type]*i);
                        gen_expr(state, file, node->value.var.value.data[i]);                                                                                            
                        gen_push(state, file, data_type_s[node->value.var.type]);
                        gen_write(state, file);
                    }
                } else if(node->value.var.is_struct) {
                    Node cur_struct = get_struct(structs, node->value.var.struct_name);
                    if(cur_struct.value.structs.values.count != node->value.var.struct_value.count) {
                        PRINT_ERROR(node->loc, "struct value count does not match\n");
                    }
                    size_t alloc_s = 0;
                    for(size_t i = 0; i < cur_struct.value.structs.values.count; i++) {
                        alloc_s += data_type_s[cur_struct.value.structs.values.data[i].value.var.type];
                    }
                    gen_struct_alloc(state, file, alloc_s);
                    for(size_t i = 0; i < cur_struct.value.structs.values.count; i++) {
                        gen_struct_value(state, file, i, &cur_struct.value.structs.values.data[i], node);
                    }
                } else {
                    fprintf(file, "; expr\n");                                                            
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
                    PRINT_ERROR(node->loc, "variable `"View_Print"` referenced before assignment", View_Arg(node->value.var.name));
                }
                gen_inswap(file, state->stack_s-index);    
                //gen_pop(state, file);
            } break;
            case TYPE_ARR_INDEX: {
                fprintf(file, "; arr index\n");                                            
                int index = get_variable_location(state, node->value.array.name);
                if(index == -1) {
                    PRINT_ERROR(node->loc, "array `"View_Print"` referenced before assignment", View_Arg(node->value.var.name));
                }
                Type_Type type = get_variable_type(state, node->value.array.name);                                            
                gen_arr_offset(state, file, index, node->value.array.index, type);
                fprintf(file, "; expr\n");                                                                                                    
                gen_expr(state, file, node->value.array.value.data[0]);                                                    
                fprintf(file, "; size\n");                                                                                                                        
                gen_push(state, file, data_type_s[type]);
                gen_write(state, file);
            } break;
            case TYPE_FUNC_DEC: {
                fprintf(file, "; function declaration\n");                                                                                                                
                Function function = {0};
                memcpy(&function, &node->value.func_dec, sizeof(Function));
                DA_APPEND(&state->functions, function);
                DA_APPEND(&state->block_stack, BLOCK_FUNC);                
                DA_APPEND(&state->ret_stack, state->stack_s);                
                DA_APPEND(&state->scope_stack, state->stack_s);                
                for(size_t i = 0; i < function.args.count; i++) {
                    // TODO: does not push the proper stack size when using str literals
                    Variable var = {0};
                    var.stack_pos = ++state->stack_s;
                    var.name = function.args.data[i].value.var.name;
                    var.type = function.args.data[i].value.var.type;
                    DA_APPEND(&state->vars, var);    
                }
                gen_jmp(file, node->value.func_dec.label);                                
                gen_func_label(file, function.name);
            } break;
            case TYPE_FUNC_CALL: {
                Function *function = get_func(state->functions, node->value.func_call.name);
                if(!function) { 
                    PRINT_ERROR(node->loc, "function `"View_Print"` referenced before assignment\n", View_Arg(node->value.func_call.name));
                }
                if(function->args.count != node->value.func_call.args.count) {
                    PRINT_ERROR(node->loc, "args count do not match for function `"View_Print"`\n", View_Arg(function->name));
                }
                for(size_t i = 0; i < node->value.func_call.args.count; i++) {
                    gen_expr(state, file, node->value.func_call.args.data[i]);
                }
                gen_func_call(file, node->value.func_call.name);
                state->stack_s -= node->value.func_call.args.count;
                // for the return value
                if(function->type != TYPE_VOID) fprintf(file, "pop\n");
            } break;
            case TYPE_RET: {
                fprintf(file, "; return\n");
                if(state->functions.count == 0) {
                    PRINT_ERROR(node->loc, "return without function definition");
                }
                Function function = state->functions.data[state->functions.count-1];
                if(function.type == TYPE_VOID) {
                    PRINT_ERROR(node->loc, "function `"View_Print"` with return type of void returns value", View_Arg(function.name));    
                }
                size_t pos = state->ret_stack.data[state->ret_stack.count-1] + 1;
                gen_expr(state, file, node->value.expr);
                ASSERT(pos <= state->stack_s, "pos is too great");
                // TODO: need to inswap with the arr and str capacity in those cases as well
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
                ASSERT(state->block_stack.count > 0, "block stack underflowed");
                if(state->block_stack.data[--state->block_stack.count] == BLOCK_IF) {
                    gen_jmp(file, node->value.el.label2);
                } else {
                    PRINT_ERROR(node->loc, "expected `if` but found `%d`\n", state->block_stack.data[state->block_stack.count]);
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
                ASSERT(state->block_stack.count > 0, "block stack was underflowed");
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
            case TYPE_EXPR_STMT: {
                gen_expr(state, file, node->value.expr_stmt);
                if(node->value.expr_stmt->return_type != TYPE_VOID) gen_pop(state, file);
            } break;
            default:
                break;
        }    
    }

}
    
void generate(Program_State *state, Program *program, char *filename) {
    char *output = append_tasm_ext(filename);
    FILE *file = fopen(output, "w");
    gen_program(state, program->nodes, program->structs, file);
    fprintf(file, "; exit\n");                
    gen_push(state, file, 0);
    fprintf(file, "native 60\n");
    state->stack_s--;
    //gen_program(state, program->functions, program->functions, file);        
}
