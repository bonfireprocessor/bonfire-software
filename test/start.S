#include "platform.h"
#include "bonfire_gpio.h"

.section .init
.global _start
.global get_impid
.extern main
.extern _stacktop

_start:

#ifndef BONFIRE_CORE
la s1, trap
csrw mtvec, s1
#endif

la sp,_stacktop // set SP to end of RAM
jal main


get_impid:
csrr a0,mimpid
ret

trap:
li a1,GPIO_BASE
li a2, 0xff
sw a2, GPIO_OUTPUT_EN(a1)
csrr t1,mepc
sw t1, GPIO_OUTPUT_VAL(a1)
j trap

