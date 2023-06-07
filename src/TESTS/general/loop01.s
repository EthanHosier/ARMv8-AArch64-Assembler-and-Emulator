ldr x2,d0
wait:
sub x2,x2,#1
cmp x2,#0x0
b.ne wait

and x0, x0, x0
d0:
.int 0x11
.int 0


