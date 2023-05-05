
section .multiboot_header
header_start:
    dd 0xe85250d6                                                       ; multiboot2 magic number
    dd 0                                                                ; architecture: protected mode i386
    dd header_end - header_start                                        ; header length
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))     ; checksum
    ; the end tag: type = 0, size = 8
    dw 0
    dw 0
    dd 8
header_end: