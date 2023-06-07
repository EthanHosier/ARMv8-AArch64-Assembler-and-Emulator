movz x0,#1
movz x1,#5
loop:
mul x2,x1,x0
add x0,x2,#0
sub x1,x1,#1
cmp x1,#0
b.ne loop
movz x3,#0x100
str x2,[x3]

and x0, x0, x0
