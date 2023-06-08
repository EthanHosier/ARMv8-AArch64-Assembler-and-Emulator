movz w6, #0x8000, lsl #16
movk x6, #0x8000, lsl #32
orr w1, w1, w6, asr #3
and x0, x0, x0
