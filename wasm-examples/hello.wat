(module
  (import "native.libc" "printf"
    (func $printf (param i32) (result i32)))

  (memory (export "memory") 1)

  (data (i32.const 1024)
    "Hello World from Omega WASM\n")

  (func $main
    (call $printf
      (i32.const 1024)
    )
    drop
  )

  (export "_start" (func $main))
)
