st: 
ldr x0, l1
ldr x1, l2

b end

l1: 
    .int 0xf
    .int 0xa
l2: 
    .int 0xb

end:


and x0, x0, x0
