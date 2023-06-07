movz x2,#2
movz x0,#1
movz x1,#99
str x1,[x0,#0x120]
movz x0, #0x121
wait2:
sub x2,x2,#1
cmp x2,#0
str x1,[x0, #0x1]!
b.ne wait2

and x0, x0, x0
