# TIM - Titanium Virtual Machine

Implementation of a virtual machine in C.

VM currently has 29 instructions as well as a couple native functions. List can be found in tim.h.
There is also a working assembly which contains support for all instructions in the VM. 

Example of hello world in assembly:
```asm
@imp "stddefs.tash"

push_str "Hello, world!\n"
get_str 0 ; Index of the string on the data stack
push STDOUT
write ; length is inferred because the string is null-terminated
```
