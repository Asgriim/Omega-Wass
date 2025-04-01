(module
  ;; импорт printf
  (import "native" "printf" (func $printf (param i32) (result i32)))

  ;; память на 1 страницу = 64KB
  (memory (export "memory") 1)

  ;; строка "Hello World from Omega WASM\00" по адресу 1024
  (data (i32.const 1024) "Hello World from Omega WASM\00")

  (func $main
    ;; передаём указатель на строку в printf
    (call $printf
      (i32.const 1024)
    )
    drop ;; удалить возвращаемое значение (int от printf)
  )

  ;; экспортируем _start
  (export "_start" (func $main))
)
