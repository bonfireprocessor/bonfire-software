#include "platform.h"
#include "bonfire_gpio.h"

.section .init
.global _start

#define SIM

_start:

li a1,GPIO_BASE

#ifndef BONFIRE_CORE
la s1, trap
csrw mtvec, s1
#endif

# _write_word((void*)GPIO_BASE+GPIO_OUTPUT_EN,0xf);
li a2, 0xff
sw a2, GPIO_OUTPUT_EN(a1)
# _write_word((void*)GPIO_BASE+GPIO_OUTPUT_VAL,0x9);

#ifdef BONFIRE_CORE
li a2,0x9
#else
csrr a2, mimpid
#endif
sw a2, GPIO_OUTPUT_VAL(a1)

li t1,5
mul a2,t1,t1
sw a2, GPIO_OUTPUT_VAL(a1)


#ifdef SIM
li a2,0xaa
#else
li a2,0xaa00000
#endif

loop:
add a2,a2,-1

#ifdef SIM
mv a3,a2
#else
srli a3,a2,20
#endif

sw a3, GPIO_OUTPUT_VAL(a1)
#nop
bnez a2, loop

li a3,0x55
sw a3, GPIO_OUTPUT_VAL(a1)

endless:
j _start
# should not be executed
li a3,0xff
sw a3, GPIO_OUTPUT_VAL(a1)

trap:
csrr t1,mepc
sw t1, GPIO_OUTPUT_VAL(a1)
j trap
