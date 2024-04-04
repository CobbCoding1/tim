#define ARENA_IMPLEMENTATION
#include "main.h"

#define TIM_IMPLEMENTATION
#define TIM_H
#include "tim.h"

void usage(char *file) {
    fprintf(stderr, "usage: %s <filename.cano>\n", file);
    exit(1);
}
    
void *custom_realloc(void *ptr, size_t size) {
    void *new_ptr = realloc(ptr, size);
    ASSERT(new_ptr != NULL, "Out of memory, maybe close some programs? Alternatively you could buy some more RAM.");
    ptr = new_ptr;
    return new_ptr;
}
	
int main(int argc, char *argv[]) {
    if(argc != 2) {
        usage(argv[0]);
    }
    char *filename = argv[1];
	Arena token_arena = arena_init(sizeof(Token)*ARENA_INIT_SIZE);	
    String_View view = read_file_to_view(&token_arena, filename);
	Arena string_arena = arena_init(sizeof(char)*ARENA_INIT_SIZE);
    Token_Arr tokens = lex(&token_arena, &string_arena, filename, view);
    Blocks block_stack = {0};
	Arena node_arena = arena_init(sizeof(Node)*ARENA_INIT_SIZE);
    Program program = parse(&node_arena, tokens, &block_stack);
	
	arena_free(&token_arena);	
	
    Program_State state = {0};
	state.program = program;
    state.structs = program.structs;
    generate(&state, &program, filename);
	
	state.machine.program_size = state.machine.instructions.count;
	write_program_to_file(&state.machine, "out.tim");	
	
	arena_free(&node_arena);
	arena_free(&string_arena);
	free(state.vars.data);
	free(state.functions.data);
	free(state.scope_stack.data);
	free(state.block_stack.data);
	free(state.ret_stack.data);
	free(state.while_labels.data);
}
