ldr x2,d0
add x1,x2,#2
cmp  x3,#2
b.ne l0
add x1,x2,#5
l0:
    and x0,x0,x0
d0:
  .int 0x20200020
  .int 0
.int 0

