set -xe
mkdir -p build/obj

OBJ="build/obj"

gcc -c src/main.c -o build/obj/main.o
gcc -c src/frontend.c -o build/obj/frontend.o
gcc -c src/backend.c -o build/obj/backend.o
gcc -c src/view.c -o build/obj/view.o
gcc $OBJ/main.o $OBJ/frontend.o $OBJ/backend.o $OBJ/view.o -o build/main
