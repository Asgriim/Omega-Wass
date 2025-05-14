#include <wasi/api.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#define MES "==OMEGA WASSS COOL=="
int main() {
    uint64_t start, end;
    __wasi_clock_time_get(1 /*CLOCK_REALTIME*/, 0 /*precision*/, &start);

    int fd = open("aboba.txt", O_CREAT|O_WRONLY|O_TRUNC, 0755);
    for (int i = 0; i < 100000; ++i) {
        write(fd, MES, sizeof(MES));
    }
    close(fd);

    __wasi_clock_time_get(1, 0, &end);
    printf("exec time = %llu \n", (unsigned long long)(end - start) / 100);
    return 0;
}