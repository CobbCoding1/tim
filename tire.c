#include "tire.h"

int main(int argc, char *argv[]){
    if(argc < 2){
        fprintf(stderr, "Usage: %s <file_name.tim>\n", argv[0]);
        exit(1);
    }

    char *file_name = argv[1];
    Machine *machine = malloc(sizeof(Machine));
    machine = read_program_from_file(machine, file_name);
    run_instructions(machine);
    //print_stack(machine);
}
