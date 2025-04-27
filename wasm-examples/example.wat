;; example.wat
(module
  ;; Тип для косвенных вызовов (call_indirect) убран

  ;; Память, таблица и изменяемая глобальная переменная
  (memory 1)
  (table 1 funcref)
  (global $g (mut i32) (i32.const 42))

  ;; 1) Константы (i-const, f-const)
  (func $consts
    (i32.const 10)   drop
    (i64.const 20)   drop
    (f32.const 3.14) drop
    (f64.const 6.28) drop
  )

  ;; 2) Локальные (get/set/tee) + арифметика
  (func $locals (param $x i32)
    (local $tmp i32)
    (local.set $tmp (local.get $x))
    (local.tee $tmp (i32.add (local.get $tmp) (i32.const 1)))
    drop
  )

  ;; 3) Глобалы (get/set)
  (func $globals
    (global.set $g (i32.const 7))
    (global.get $g) drop
  )

  ;; 4) Таблица (get/set + ref.func)
  (elem (i32.const 0) $consts)
  (func $table_ops (param $i i32)
    (table.set  (local.get $i) (ref.func $consts))
    (table.get  (local.get $i)) drop
  )

  ;; 5) Память (load/store разных типов)
  (func $memory_ops
    (i32.store (i32.const 0)  (i32.const 123))
    (i64.store (i32.const 8)  (i64.const 456))
    (f32.store (i32.const 16) (f32.const 1.23))
    (f64.store (i32.const 24) (f64.const 4.56))
    (f64.load  (i32.const 24)) drop
  )

  ;; 6) Управление потоком: block, loop, br, br_if, if/else
  (func $control (param $i i32)
    (block $B1
      (br_if $B1 (local.get $i))
    )
    (loop $L1
      (br_if $L1 (i32.eqz (local.get $i)))
    )
    (if (i32.gt_s (local.get $i) (i32.const 0))
      (then (nop))
      (else (nop))
    )
  )

  ;; 7) Прямой вызов
  (func $direct_call (param $v i32)
    (call $control (local.get $v))
  )

  ;; call_indirect удалён — больше нет функции indirect_call
)

