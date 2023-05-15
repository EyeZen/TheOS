#include "IDT.h"

#include "Logging.h"
#include "print.h"
#include <stddef.h>

extern char vector_0_handler[];

const int num_idt_descriptors = 256;
struct interrupt_descriptor idt[256];


void set_idt_entry(uint8_t vector, void(*handler)(void), uint8_t dpl)
{
    struct interrupt_descriptor* entry = &idt[vector];
    //trap gate + present + DPL
    entry->flags = 0b1110 | ((dpl & 0b11) << 5) |(1 << 7);
    //ist disabled
    entry->ist = 0;
    entry->selector = 0x8;
    entry->address_low = (uint16_t) ((uint64_t)handler&0xFFFF);;
    entry->address_mid = (uint16_t) ((uint64_t)handler >> 16);
    entry->address_high = (uint32_t)((uint64_t)handler>> 32);
    entry->reserved = 0x0;
}

void load_idt()
{
    struct idtr idt_reg;
    idt_reg.limit = num_idt_descriptors * sizeof(struct interrupt_descriptor) - 1;
    idt_reg.base = (uint64_t)&idt;
    asm volatile("lidt %0": :"g" (idt_reg));
}

void init_idt() 
{
	for (size_t i = 0; i < num_idt_descriptors; i++) {
		set_idt_entry(i, (void*)(vector_0_handler + (i * 16)), 0);
	}

	load_idt();
}


void interrupt_dispatch(struct cpu_status_t* context)
{
    log_interrupt(context);
    log_heap();
    kprintf("\nHalted");
    while(1) asm volatile("hlt");
    switch(context->vector_number) {
        case INTR_DIVIDE_ERROR: {

        } break;
        case INTR_DEBUG_EXC: {

        } break;
        case INTR_NMI_INTERRUPT: {

        } break;
        case INTR_BREAKPOINT: {

        } break;
        case INTR_OVERFLOW: {

        } break;
        case INTR_BOUND_RANGE_EXCEED: {

        } break;
        case INTR_INVALID_OPCODE: {

        } break;
        case INTR_DEV_NOT_AVL: {

        } break;
        case INTR_DOUBLE_FAULT: {

        } break;
        case INTR_COPROC_SEG_OVERRUN: {

        } break;
        case INTR_INVALID_TSS: {

        } break;
        case INTR_SEGMENT_NOT_PRESENT: {

        } break;
        case INTR_STACK_SEGMENT_FAULT: {

        } break;
        case INTR_GENERAL_PROTECTION: {

        } break;
        case INTR_PAGE_FAULT: {

        } break;
        case INTR_INT_RSV: {

        } break;
        case INTR_FLOATING_POINT_ERR: {

        } break;
        case INTR_ALIGNMENT_CHECK: {

        } break;
        case INTR_MACHINE_CHECK: {

        } break;
        case INTR_SIMD_FP_EXC: {

        } break;
        case INTR_APIC_TIMER_INTERRUPT: {

        } break;
        case INTR_KEYBOARD_INTERRUPT: {

        } break;
        case INTR_PIT_INTERRUPT: {

        } break;
        case INTR_APIC_SPURIOUS_INTERRUPT: {
            
        } break;
        default: {

        }
    }
}