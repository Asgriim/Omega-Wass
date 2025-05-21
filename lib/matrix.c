#include <stdio.h>

void print_border(int rows, int cols, int width) {
    for (int j = 0; j < cols; j++) {
        putchar('+');
        for (int k = 0; k < width; k++) putchar('-');
    }
    puts("+");
}
void print_matrix(int rows, int cols) {
    const int width = 4;  // ширина поля для числа
    // Функция для печати горизонтальной линии рамки длиной cols*width


    print_border(rows, cols, width);
    for (int i = 0; i < rows; i++) {
        // печать одной строки с вертикальными границами
        for (int j = 0; j < cols; j++) {
            printf("|%*d", width, i * cols + j);
        }
        puts("|");
        print_border(rows, cols, width);
    }
}