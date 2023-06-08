movz x5, #11559
movk x5, #0xe, lsl #16
movz x17, #0
movk x17, #0x8a00, lsl #16
str x17, [x5], #0
br x5
and x0, x0, x0

