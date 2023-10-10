set -xe
gcc tim.c time.c -o time -Wall -Wextra
gcc tasm.c  tasmlexer.c tasmparser.c tim.c -o tasm -Wall -Wextra
