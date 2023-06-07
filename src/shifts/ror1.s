movz x6, #0x1234
movk w6, #0x8000, lsl #16
movk x6, #0xabcd, lsl #32
orr w1, w1, w6, ror #3
and x0, x0, x0
