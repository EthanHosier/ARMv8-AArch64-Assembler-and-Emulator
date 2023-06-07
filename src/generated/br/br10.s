movz x8, #19719
movk x8, #0x1, lsl #16
movz x16, #0
movk x16, #0x8a00, lsl #16
str x16, [x8], #0
br x8
and x0, x0, x0

