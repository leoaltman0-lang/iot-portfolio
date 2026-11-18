ENTRY(Reset_Handler)

MEMORY {
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

__stack_top = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS {
    .isr_vector : { KEEP(*(.isr_vector)) } > FLASH
    .text : {
        *(.text .text.*)
        *(.rodata .rodata.*)
    } > FLASH
    .ARM.exidx : { *(.ARM.exidx*) } > FLASH
    .data : {
        . = ALIGN(4);
        __data_start = .;
        *(.data .data.*)
        . = ALIGN(4);
        __data_end = .;
    } > RAM AT> FLASH
    __data_load = LOADADDR(.data);
    .bss (NOLOAD) : {
        . = ALIGN(4);
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } > RAM
}
