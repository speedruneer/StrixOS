OUTPUT_FORMAT(elf32-i386)
ENTRY(entry_start)

SECTIONS
{
    . = 0x9000;  /* bootloader load address */

    .text ALIGN(4K) :
    {
        *(.text)
    }

    .data ALIGN(4K) :
    {
        *(.data)
    }

    .rodata ALIGN(4K) :
    {
        *(.rodata)
    }

    .bss ALIGN(4K) :
    {
        *(.bss)
        *(COMMON)
    }
    _heap_start = .;    /* heap starts here */
    . = . + 0xA00000;   /* 10 MB */
    _heap_end = .;      /* heap starts here */
}

/*
 * Yes, totally fine, 10MB heap, no I don't think that's safe, especially when I wanted a 100MB one
 */
