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
    String_View view = read_file_to_view(filename);
    Token_Arr tokens = lex(filename, view);
    //print_token_arr(tokens);
    Blocks block_stack = {0};
    Program program = parse(tokens, &block_stack);
    Program_State state = {0};
    generate(&state, &program, filename);
}
