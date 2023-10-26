# TIM - Titanium Virtual Machine

Implementation of a virtual machine in C.

VM currently has 29 instructions. List can be found in tim.h.
There is also a working assembly which contains support for all instructions in the VM. 

Example of hello world in assembly:
```asm
@imp "stddefs.tash"

push "Hello, world!\n"
write STDOUT 14
```
