
add w10, w4, #0xb2b, lsl #12
cmp w10, w4

b.gt l9
add w17, w17, #206, lsl #0
l9:
add w17, w17, #0xff5, lsl #12
and x0, x0, x0

