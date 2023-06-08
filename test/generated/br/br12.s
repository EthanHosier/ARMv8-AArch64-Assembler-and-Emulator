movz x6, #62941
movk x6, #7, lsl #16
movz w16, #0
movk w16, #0x8a00, lsl #16
str w16, [x6], #0
br x6
and x0, x0, x0

