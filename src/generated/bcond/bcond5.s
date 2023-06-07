
cmp x18, x18
add x26, x18, #391, lsl #12
cmp x18, x26

b.le l5
add x22, x22, #0x4ce, lsl #12
l5:
add x22, x22, #3310, lsl #12
and x0, x0, x0

