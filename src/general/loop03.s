movz x2,#10
wait2:
sub x2,x2,#1
cmp x2,#0
b.ne wait2

and x0, x0, x0
