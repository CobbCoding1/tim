set -xe
gcc tim.c tasmlexer.c tasm.c -o tim -Wall -Wextra
gcc tasm.c tasmlexer.c -o tasm -Wall -Wextra
