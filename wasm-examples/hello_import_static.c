#include <stdio.h>

void aboba_print() {
    printf("start %d \n", 1);
}

void aboba_print2() {
    aboba_print();
    printf("start2 %d \n", 1);
}

int main() {
    aboba_print2();
    for (int i = 0; i < 1000; ++i) {
        if (i % 2 == 0)
            printf("%d : even \n", i);
        else
            printf("%d : odd \n", i);
    }
    return 0;
}

