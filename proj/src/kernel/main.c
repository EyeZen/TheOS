#include "print.h"
#include "multiboot.h"
#include "utils.h"
#include "Logging.h"

#include <stdint.h>

struct interrupt_descriptor
{
    uint16_t address_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t flags;
    uint16_t address_mid;
    uint32_t address_high;
    uint32_t reserved;
} __attribute__((packed));

const int num_idt_descriptors = 256;
struct interrupt_descriptor idt[256];

void set_idt_entry(uint8_t vector, void* handler, uint8_t dpl)
{
    uint64_t handler_addr = (uint64_t)handler;

    struct interrupt_descriptor* entry = &idt[vector];
    entry->address_low = handler_addr & 0xFFFF;
    entry->address_mid = (handler_addr >> 16) & 0xFFFF;
    entry->address_high = handler_addr >> 32;
    //your code selector may be different!
    entry->selector = 0x8;
    //trap gate + present + DPL
    entry->flags = 0b1110 | ((dpl & 0b11) << 5) |(1 << 7);
    //ist disabled
    entry->ist = 0;
}

struct idtr
{
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

void load_idt(void* idt_addr)
{
    struct idtr idt_reg;
    idt_reg.limit = 0xFFF;
    idt_reg.base = (uint64_t)idt_addr;
	uint64_t reg_addr = (uint64_t)&idt_reg;
    asm volatile("lidt %0" :: "m"(reg_addr));
}

extern char vector_0_handler[];

void intr_handler()
{
	log_str("interr handler");
}

void init_idt() 
{
	log_astr("init_idt"); log_endl();
	for (size_t i = 0; i < num_idt_descriptors; i++) {
		// set_idt_entry(i, (void*)(vector_0_handler + (i * 16)), 0);
		set_idt_entry(i,(void*)intr_handler, 0);
	}

	load_idt((void*)idt);
}

struct cpu_status_t
{
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;

    uint64_t vector_number;
    uint64_t error_code;

    uint64_t iret_rip;
    uint64_t iret_cs;
    uint64_t iret_flags;
    uint64_t iret_rsp;
    uint64_t iret_ss;
};

void interrupt_dispatch(struct cpu_status_t* context)
{
	log_astr("interrupt dispatch: ");
	log_anum(context->vector_number);
    switch (context->vector_number)
    {
        case 13:
            log_astr("general protection fault");
            break;
        case 14:
            log_astr("page fault.");
            break;
        default:
            log_astr("unexpected interrupt.");
            break;
    }
}

// called inside main64.asm
void kernel_main(struct multiboot_info_header* mboot_header) {
    print_clear();
    print_set_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);
    print_str("Welcome to our 64-bit kernel\n");
	
	init_serial();
	log_mbheader(mboot_header);

	init_idt();
	char* val = (char*)0x200000;
	print_str("\nMemory Byte: ");
	print_num((uint32_t)*val);
	*val = 20;
	print_str("\nMemory Byte: ");
	print_num((uint32_t)*val);
}
