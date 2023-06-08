movz x2, #0xf141
movk x2, #1, lsl #16
movz x14, #0
movk x14, #35328, lsl #16
str x14, [x2], #0
br x2
and x0, x0, x0

