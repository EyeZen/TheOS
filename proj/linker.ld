ENTRY(start)

SECTIONS
{
    . = 1M;

    _kernel_start = .;
    .boot BLOCK(4K) : ALIGN(4K)
    {
        KEEP(*(.multiboot_header))
    }

    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.text)
    }

    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(.bss)
    }

    _kernel_end = .;
}