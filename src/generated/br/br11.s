movz x30, #0x1b99
movk x30, #3, lsl #16
movz w6, #0x0
movk w6, #35328, lsl #16
str w6, [x30], #0
br x30
and x0, x0, x0

