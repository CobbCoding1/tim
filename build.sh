set -xe
gcc tim.c tire.c -o tire -Wall -Wextra -ggdb 
gcc tasm.c  tasmlexer.c tasmparser.c tim.c -o tasm -Wall -Wextra 


while getopts "tw" flag; do
 case $flag in
   t)
        cd tests
        for file in *.tasm
        do
            ../tasm $file
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
