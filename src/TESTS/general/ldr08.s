ldr x0,d0
ldr x2,d1
add x2,x2,#8

and x0,x0,x0

d0:
  .int 0x20200020
  .int 0
.int 0
d1:
  .int 0x20200028
  .int 0

