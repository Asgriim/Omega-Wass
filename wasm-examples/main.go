package main

//go:wasmimport libmatx.so.1 print_matrix(II)
func print_matrix(rows, cols int32)

//export _start
func main() {
    print_matrix(5, 4)
}

