// wasm_matrix.c

// импорт из модуля "matrix" функции print_matrix(II)
__attribute__((import_module("libmatx.so.1")))
__attribute__((import_name("print_matrix(II)")))
void print_matrix(int rows, int cols);

// экспортируемая точка входа
void _start() {
    // вызов с параметрами, например 5×4
    print_matrix(5, 4);
}
