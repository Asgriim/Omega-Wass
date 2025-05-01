__attribute__((import_module("libc")))
__attribute__((import_name("printf(*)")))
int printf(const char* fmt);

__attribute__((import_module("libc")))
__attribute__((import_name("printf(*I)")))
int my_printf(const char* fmt, int i);

void aboba_print() {
    my_printf("start %d \n", 1);
}

void aboba_print2() {
    aboba_print();	
    my_printf("start2 %d \n", 1);
}



void _start() {  // <-- имя функции прямо _start, без alias
    aboba_print2();
    for (int i = 0; i < 10; ++i) {
        if (i % 2 == 0)
            printf("even\n");
        else
            printf("odd\n");
    }
}

