/*
    GNU linker script for Ambiq Apollo 4+
*/

/* Specify the memory areas */
MEMORY
{
    FLASH (rx)      : ORIGIN = 0x00018000, LENGTH = 1952K /* entire flash */
    RAM (xrw)       : ORIGIN = 0x10000000, LENGTH = 384K  /* TCM */
}

/* stack extents */
_sstack = ORIGIN(RAM) + LENGTH(RAM) - 8K;
_estack = ORIGIN(RAM) + LENGTH(RAM);

/* define output sections */
SECTIONS
{
    /* The program code and other data goes into FLASH */
    .text :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector)) /* isr vector table */
        *(.text*)          /* .text* sections (code) */
        *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
        . = ALIGN(4);
        _etext = .;        /* define a global symbol at end of code */
    } >FLASH

    /* For C++ exception handling */
    .ARM :
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } >FLASH

    /* Used by the start-up code to initialise data */
    _sidata = LOADADDR(.data);

    /* Initialised data section, start-up code will copy it from flash to RAM */
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } >RAM AT> FLASH

    /* Zeroed-out data section */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } >RAM
}
