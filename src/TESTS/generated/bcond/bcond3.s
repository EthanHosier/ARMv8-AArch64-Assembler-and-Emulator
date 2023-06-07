
add w11, w8, #1, lsl #0
add w8, w8, #2, lsl #0
cmp w8, w11

b.ne l3
add w4, w4, #0x3ba, lsl #0
l3:
add w4, w4, #0x4ce, lsl #12
and x0, x0, x0

