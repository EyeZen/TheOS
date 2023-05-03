ENTRY(start)

SECTIONS
{
    . = 1M;

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
}