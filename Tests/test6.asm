; find perimeter of a rectange with given length and breadth
ldc 0x1000
a2sp
ldc values
ldnl 0
ldc 1
shl
adj -1
stl 0
ldc values
ldnl 1
ldc 1
shl
ldl 0
add ; should produce 0x6c (108) as the answer.
HALT
values:
data 42
data 12