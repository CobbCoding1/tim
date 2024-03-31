#include "tire.h"

void shift(int *argc, char ***argv) {
	*argc += 1;	
	*argv += 1;
}
	
void print_usage(char *program) {
    fprintf(stderr, "Usage: %s --<flags> <file_name.tim>\n", program);
    fprintf(stderr, "Flags: disasm\n");		
    exit(1);
}

int main(int argc, char **argv){
	int disasm = 0;
	char *program = *argv;	
	shift(&argc, &argv);
	char *flag = *argv;	
	shift(&argc, &argv);	
    if(flag == NULL) {
		print_usage(program);
    }
	
	char *filename = NULL;
	
	if(strncmp(flag, "--dis", 5) == 0) {
		disasm = 1;
		filename = *argv;
		if(filename == NULL) print_usage(program);	
	} else {
		filename = flag;	
	}
		
	assert(filename);
	
    Machine *machine = calloc(1, sizeof(Machine));
    machine = read_program_from_file(machine, filename);
	if(disasm) {
		machine_disasm(machine);
		return 0;
	}
    run_instructions(machine);
	return 0;
}
