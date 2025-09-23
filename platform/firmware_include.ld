ENTRY(_start)

PHDRS
{
  text PT_LOAD FLAGS(5); /* R(4) + X(1) = 5 */
  rodata PT_LOAD FLAGS(4); /* R(4)  */
  data PT_LOAD FLAGS(6); /* R(4) + W(2) = 6 */
}


SECTIONS
{
  . = ORIGIN(RAM);
  _rombase = .;

  /* Code  */
  .text : ALIGN(4)
  {
    KEEP(*(.init))
    *(.text .text.*)
  } > RAM : text

   /* rodata: Read-only data */
  .rodata : ALIGN(4)
  {
    *(.rdata)
    *(.rodata)
    *(.rodata.*)
    *(.gnu.linkonce.r.*)
  } > RAM : rodata

  /* Writable data (read + write) */
  .data : ALIGN(4)
  {
    *(.data .data.*)
    *(.sdata .sdata.*)
    *(.gnu.linkonce.d.*)
    *(.srodata.*)            /* small readonly goes to data if you prefer RW */
    *(.eh_frame)             /* keep if needed */
    _edata = .;
  } > RAM :data

  /* Zero-initialized (no load image), same RW segment */
  .bss (NOLOAD) : ALIGN(4)
  {
    _fbss = .;
    *(.sbss .sbss.*)
    *(.gnu.linkonce.sb.*)
    *(.bss .bss.*)
    *(COMMON)
    end = .; _end = .; __end = .;
  } > RAM :data

  /* Stack top at end of RAM (16-byte aligned) */
  _stacktop = ALIGN(16, ORIGIN(RAM) + LENGTH(RAM)) - 16;
}