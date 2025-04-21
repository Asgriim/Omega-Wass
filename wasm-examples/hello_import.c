__attribute__((import_module("env")))
__attribute__((import_name("printf")))
int printf(const char* fmt);

void _start() {  // <-- имя функции прямо _start, без alias
    for (int i = 0; i < 10; ++i) {
        if (i % 2 == 0)
            printf("even\n");
        else
            printf("odd\n");
    }
}

