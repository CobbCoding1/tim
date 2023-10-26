set -xe
gcc -lm tim.c tire.c -o tire -Wall -Wextra -ggdb 
gcc -lm tasm.c  tasmlexer.c tasmparser.c tim.c -o tasm -Wall -Wextra 

while getopts "t" flag; do
 case $flag in
   t)
       for file in tests/*.tasm
       do
           ./tasm $file
       done
   ;;
   \?)
   ;;
 esac
done
