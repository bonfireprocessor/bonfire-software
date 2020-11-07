MEMORY
{
    RAM (rwx)    : ORIGIN = 0xC0000000, LENGTH = 16384
}


ENTRY(_start)


SECTIONS
{
  _rombase = 0xC0000000;

  . = 0x0;

  .text : {
    *(.init)
    *(.text.*)

   } > RAM

  .data  ALIGN(4) : {
     *(.rodata.*)
     *(.rodata)
     *(.eh_frame)
     *(.sdata)
     *(.sdata.*)
     *(.srodata.*)
     *(.gnu.linkonce.s.*)
     *(.data)
     *(.data.*)
     *(.gnu.linkonce.d.*)
     _edata = .;
  } > RAM

  .bss ALIGN(4): {
     _fbss = .;
     *(.sbss)
     *(.sbss.*)
     *(.gnu.linkonce.sb.*)
     *(.bss)
     *(.bss.*)
     *(.gnu.linkonce.b.*)
     *(COMMON)

      end = .; _end = .; __end = .;

 } > RAM
  _stacktop =  _rombase + 0x3ff0;

}

