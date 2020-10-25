#include "platform.h"
#include "bonfire_gpio.h"

.section .init
.global _start

_start:

li a1,GPIO_BASE

#_write_word((void*)GPIO_BASE+GPIO_OUTPUT_EN,0xf);
li a2, 0xff
sw a2, GPIO_OUTPUT_EN(a1)
#_write_word((void*)GPIO_BASE+GPIO_OUTPUT_VAL,0x9);
li a2,0x9
sw a2, GPIO_OUTPUT_VAL(a1)

li a2,0xaa00000
loop:
add a2,a2,-1
srli a3,a2,20
sw a3, GPIO_OUTPUT_VAL(a1)
nop
bnez a2, loop

li a3,0x55
sw a3, GPIO_OUTPUT_VAL(a1)

endless:
j _start
# should not be executed
li a3,55
sw a3, GPIO_OUTPUT_VAL(a1)






