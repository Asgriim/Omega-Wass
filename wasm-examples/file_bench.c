__attribute__((import_module("libc")))
__attribute__((import_name("open(*II)")))
int open(const char* file, int flag, int mode);

__attribute__((import_module("libc")))
__attribute__((import_name("write(I*I)")))
int write(int fd, const void *buf, int sz);

__attribute__((import_module("libc")))
__attribute__((import_name("close(I)")))
void close(int fd);

__attribute__((import_module("libc")))
__attribute__((import_name("printf(*I)")))
int printf(const char* fmt, int i);

#include "time.h"
#define MES "==OMEGA WASSS COOL=="
void _start() {
    int start = clock();
    int fd = open("aboba.txt", O_CREAT | O_WRONLY | O_TRUNC, 0755);
//    printf("fd = %d\n", fd);

    for (int i = 0; i < 100000; ++i) {
        write(fd, MES, sizeof(MES) - 1);
    }
    close(fd);
    int end = clock();
    printf("exec time = %d\n", end - start);
}

