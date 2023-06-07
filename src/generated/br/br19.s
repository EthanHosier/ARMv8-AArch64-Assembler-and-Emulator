movz x17, #0x217f
movk x17, #8, lsl #16
movz x29, #0x0
movk x29, #0x8a00, lsl #16
str x29, [x17], #0
br x17
and x0, x0, x0

