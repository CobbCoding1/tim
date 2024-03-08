set -xe
mkdir -p build

gcc -o build/main src/main.c src/frontend.c src/backend.c
return
gcc -c src/frontend.c -o build/frontend.o
gcc -c src/backend.c -o build/backend.o
gcc -c src/main.c -o build/main.o
gcc -o build/main build/main.o build/impl.o build/frontend.o build/backend.o -Wall -Wextra -ggdb2

