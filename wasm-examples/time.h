
#ifndef OWASM_VM_TIME_H
#define OWASM_VM_TIME_H
__attribute__((import_module("libc")))
__attribute__((import_name("clock()")))
int clock();
#define O_CREAT          0100
#define O_WRONLY	     01
#define O_TRUNC	         01000
#endif //OWASM_VM_TIME_H
