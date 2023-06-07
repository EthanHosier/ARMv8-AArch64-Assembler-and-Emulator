
cmp w0, w0
add w9, w0, #0x38d, lsl #12
cmp w9, w0

b.ge l1
add w28, w28, #0x509, lsl #12
l1:
add w28, w28, #3366, lsl #12
and x0, x0, x0

