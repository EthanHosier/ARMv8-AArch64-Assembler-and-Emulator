movz x27, #0x776e
movk x27, #0x6, lsl #16
movz w1, #0x0
movk w1, #0x8a00, lsl #16
str w1, [x27], #0
br x27
and x0, x0, x0

