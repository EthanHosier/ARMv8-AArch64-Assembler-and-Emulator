movz x10, #0xe520
movk x10, #11, lsl #16
movz x27, #0
movk x27, #0x8a00, lsl #16
str x27, [x10], #0
br x10
and x0, x0, x0

