movz x2,#5
wait:
sub x2,x2,#1
movz x1,#10
wait1:
sub x1,x1,#1
cmp x1,x2
b.ne wait1
cmp x2,#0
b.ne wait


and x0, x0, x0
