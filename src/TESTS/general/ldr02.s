movz x0,#0x02
ldr x2,[x0]
movz x0, #0x300, lsl #16
cmp x2,x0

and x0, x0, x0
