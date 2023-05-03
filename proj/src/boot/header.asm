
section .multiboot_header
header_start:
    ; magic number
    dd 0xe85250d6   ; multiboot2
    ; architecture
    dd 0 ; protected mode i386
    ; header length
    dd header_end - header_start
    ; checksum
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

    ; framebuffer tag: type=5
framebuffer_tag_start:
    dw 5
    dw 1
    dd (framebuffer_tag_end - framebuffer_tag_start)
    ; preferred width, height, bpp
    ; leave as zero to indicate "don't care"
    dd 0
    dd 0
    dd 0
framebuffer_tag_end:
    ; the end tag: type = 0, size = 8
    dd 0
    dd 0
header_end: