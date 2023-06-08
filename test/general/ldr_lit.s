st:
ldr x0, value

b end

value:
    .int 0xace0f


end:

and x0, x0, x0
