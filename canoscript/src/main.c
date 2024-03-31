#define ARENA_IMPLEMENTATION
#include "main.h"

void usage(char *file) {
    fprintf(stderr, "usage: %s <filename.cano>\n", file);
    exit(1);
}
    
void *custom_realloc(void *ptr, size_t size) {
    void *new_ptr = realloc(ptr, size);
    ASSERT(new_ptr != NULL, "buy more ram");
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
    Token_Arr tokens = lex(&token_arena, filename, view);
    Blocks block_stack = {0};
	Arena node_arena = arena_init(sizeof(Node)*ARENA_INIT_SIZE);
    Program program = parse(&node_arena, tokens, &block_stack);
    Program_State state = {0};
	state.program = program;
    state.structs = program.structs;
    generate(&state, &program, filename);
}
