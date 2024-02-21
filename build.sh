SRC="src"
OUT="build"

set -xe
gcc $SRC/tim.c $SRC/tire.c -o $OUT/tire -Wall -Wextra -ggdb
gcc $SRC/tasm.c $SRC/tasmlexer.c $SRC/tasmparser.c $SRC/tim.c -o $OUT/tasm -Wall -Wextra -ggdb


while getopts "tw" flag; do
 case $flag in
   t)
        cd tests
        for file in *.tasm
        do
            ../build/tasm $file
        done
   ;;
    w)
        i686-w64-mingw32-gcc -lm tim.c tire.c -o tire -Wall -Wextra
        i686-w64-mingw32-gcc -lm tasm.c tasmlexer.c tasmparser.c tim.c -o tasm -Wall -Wextra
   ;;
   \?)
   ;;
 esac
done
