l0: 
    ldr x1, [x0], #2
    ldr x2, l0
    ldr x0, d1
    add x0, x0, #2
    b l1
l1: 
    ldr x0, d2
    b end
d1: 
    .int 0x1
d2: 
    .int 0x2

hang:
    b hang

end:
    and x0, x0, x0
