MEMORY
{
    FLASH (rx) : ORIGIN = 0x10000000, LENGTH = 4M
    RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 520K
}

__stack_top = ORIGIN(RAM) + LENGTH(RAM);

ENTRY(_RESET_Handler);
SECTIONS
{
    .vector_table : ALIGN(512) { KEEP(*(.vector_table)) } > FLASH
    .boot_block : { KEEP(*(.boot_block)) } > FLASH

    .text : {
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    .data : {
        . = ALIGN(4);
        __data_start = .;
        *(.data*)
        . = ALIGN(4);
        __data_end = .;
    } > RAM AT> FLASH
    __data_lma = LOADADDR(.data);

    .bss (NOLOAD) : {
        . = ALIGN(4);
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } > RAM

    /DISCARD/ : { *(.ARM.exidx*) *(.ARM.attributes) }
}
