movz x6, #0x1234
movk w6, #0x8000, lsl #16
movk x6, #0x8000, lsl #32
movk x6, #0x8000, lsl #48
orr x1, x1, x6, ror #3
mov w6, w6
and x0, x0, x0
