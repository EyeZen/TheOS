global long_mode_start
extern kernel_main

section .text
bits 64
long_mode_start:
    ; load 0 into data-segment registers
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call kernel_main
    hlt ; stop processing and wait for interrupts