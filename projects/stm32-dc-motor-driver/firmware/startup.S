.syntax unified
.cpu cortex-m3
.thumb

.section .isr_vector, "a", %progbits
.word __stack_top
.word Reset_Handler
.rept 62
.word fault_stop
.endr

.section .text.Reset_Handler, "ax", %progbits
.global Reset_Handler
.type Reset_Handler, %function
.thumb_func
Reset_Handler:
    ldr r0, =0xe000ed08
    ldr r1, =0x08000000
    str r1, [r0]
    dsb
    isb

    ldr r0, =__data_load
    ldr r1, =__data_start
    ldr r2, =__data_end
1:
    cmp r1, r2
    bhs 2f
    ldr r3, [r0], #4
    str r3, [r1], #4
    b 1b
2:
    ldr r1, =__bss_start
    ldr r2, =__bss_end
    movs r3, #0
3:
    cmp r1, r2
    bhs 4f
    str r3, [r1], #4
    b 3b
4:
    bl main
    b fault_stop
.size Reset_Handler, . - Reset_Handler
