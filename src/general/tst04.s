movk x1,#1
movk x2,#23
tst x1,x2
b foo
movk x3,#3
foo:
movk x4,#4

and x0, x0, x0
