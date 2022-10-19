
__top  = 0x0efffff0; /* must be aligned to a 16 Byte boundary !!! */
__load = 0x0e000000;
__stacksize = 64k;
__loadbase = 0x10000;
__user_stack = __load - 16;

INCLUDE ram_generic.ld