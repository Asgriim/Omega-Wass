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

void _start() { 
    aboba_print2();
    for (int i = 0; i < 1000; ++i) {
        if (i % 2 == 0)
            my_printf("%d : even \n", i);
        else
            my_printf("%d : odd \n", i);
    }
}

