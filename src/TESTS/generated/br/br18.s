movz x23, #46079
movk x23, #7, lsl #16
movz x10, #0x0
movk x10, #0x8a00, lsl #16
str x10, [x23], #0
br x23
and x0, x0, x0

