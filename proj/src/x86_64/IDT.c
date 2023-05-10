#include "IDT.h"

#include "Logging.h"
#include <stddef.h>

extern char vector_0_handler[];

const int num_idt_descriptors = 256;
struct interrupt_descriptor idt[256];


void set_idt_entry(uint8_t vector, void(*handler)(void), uint8_t dpl)
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

void load_idt(void* idt_addr)
{
    struct idtr idt_reg;
    idt_reg.limit = 0xFFF;
    idt_reg.base = (uint64_t)idt_addr;
	uint64_t reg_addr = (uint64_t)&idt_reg;
    asm volatile("lidt %0" :: "m"(reg_addr));
}

void init_idt() 
{
	for (size_t i = 0; i < num_idt_descriptors; i++) {
		set_idt_entry(i, (void*)(vector_0_handler + (i * 16)), 0);
	}

	load_idt((void*)idt);
}

void interrupt_dispatch(struct cpu_status_t* context)
{
	log_astr("\ninterrupt dispatch: ");
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