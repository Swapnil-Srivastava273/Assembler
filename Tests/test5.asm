; runs a loop 999 times. (In exact 2000 instructions including HALT.)
ldc -999
loop:
adc +1
brlz loop
HALT
