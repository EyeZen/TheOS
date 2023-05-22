#include "IDT.h"

#include "Logging.h"
#include "print.h"
#include <stddef.h>
#include <ACPI.h>
#include <KTimer.h>
#include <Keyboard.h>

#define KERNEL_CS 0x8
#define IDT_PRESENT_FLAG 0x80
#define IDT_INTERRUPT_TYPE_FLAG 0x0E

//extern void interrupt_service_routine_error_code_14();
// extern void interrupt_service_routine_7();
extern void interrupt_service_routine_0();
extern void interrupt_service_routine_1();
extern void interrupt_service_routine_2();
extern void interrupt_service_routine_3();
extern void interrupt_service_routine_4();
extern void interrupt_service_routine_5();
extern void interrupt_service_routine_6();
extern void interrupt_service_routine_7();
extern void interrupt_service_routine_error_code_8();
extern void interrupt_service_routine_9();
extern void interrupt_service_routine_error_code_10();
extern void interrupt_service_routine_error_code_11();
extern void interrupt_service_routine_error_code_12();
extern void interrupt_service_routine_error_code_13();
extern void interrupt_service_routine_error_code_14();
extern void interrupt_service_routine_15();
extern void interrupt_service_routine_16();
extern void interrupt_service_routine_error_code_17();
extern void interrupt_service_routine_18();
extern void interrupt_service_routine_32();
extern void interrupt_service_routine_33();
extern void interrupt_service_routine_34();
extern void interrupt_service_routine_255();

// extern char vector_0_handler[];

const int num_idt_descriptors = 256;
struct interrupt_descriptor idt[256];


void set_idt_entry_implicit(uint8_t vector, void(*handler)(void), uint8_t dpl)
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

void set_idt_entry(uint16_t idx, uint8_t flags, uint16_t selector, uint8_t ist, void (*handler)() ){
    idt[idx].flags = flags;
    idt[idx].ist = ist;
    idt[idx].selector = selector;
    idt[idx].address_low = (uint16_t) ((uint64_t)handler&0xFFFF);
    idt[idx].address_mid = (uint16_t) ((uint64_t)handler >> 16);
    idt[idx].address_high = (uint32_t)((uint64_t)handler>> 32);
    idt[idx].reserved = 0x0;
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
	// for (size_t i = 0; i < num_idt_descriptors; i++) {
	// 	set_idt_entry_implicit(i, (void*)(vector_0_handler + (i * 16)), 0);
	// }
    int i = 0;
    while (i < num_idt_descriptors){
        idt[i].flags = 0;
        idt[i].ist = 0;
        idt[i].address_high = 0;
        idt[i].address_low = 0;
        idt[i].reserved = 0;
        idt[i].address_mid = 0;
        idt[i].selector = 0;
        i++;
    }
    set_idt_entry(0x00, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG, KERNEL_CS, 0, interrupt_service_routine_0);
    set_idt_entry(0x01, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG, KERNEL_CS, 0, interrupt_service_routine_1);
    set_idt_entry(0x02, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG, KERNEL_CS, 0, interrupt_service_routine_2);
    set_idt_entry(0x03, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG, KERNEL_CS, 0, interrupt_service_routine_3);
    set_idt_entry(0x04, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG, KERNEL_CS, 0, interrupt_service_routine_4);
    set_idt_entry(0x05, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG, KERNEL_CS, 0, interrupt_service_routine_5);
    set_idt_entry(0x06, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG, KERNEL_CS, 0, interrupt_service_routine_6);
    set_idt_entry(0x07, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG, KERNEL_CS, 0, interrupt_service_routine_7);
    set_idt_entry(0x08, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG, KERNEL_CS, 0, interrupt_service_routine_error_code_8);
    set_idt_entry(0x09, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG, KERNEL_CS, 0, interrupt_service_routine_9);
    set_idt_entry(0x0A, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG, KERNEL_CS, 0, interrupt_service_routine_error_code_10);
    set_idt_entry(0x0B, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG, KERNEL_CS, 0, interrupt_service_routine_error_code_11);
    set_idt_entry(0x0C, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG, KERNEL_CS, 0, interrupt_service_routine_error_code_12);
    set_idt_entry(0x0D, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG, KERNEL_CS, 0, interrupt_service_routine_error_code_13);
    set_idt_entry(0x0E, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG, KERNEL_CS, 0, interrupt_service_routine_error_code_14);
    set_idt_entry(0x0F, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG, KERNEL_CS, 0, interrupt_service_routine_15);
    set_idt_entry(0x10, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG, KERNEL_CS, 0, interrupt_service_routine_16);
    set_idt_entry(0x11, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG, KERNEL_CS, 0, interrupt_service_routine_error_code_17);
    set_idt_entry(0x12, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG, KERNEL_CS, 0, interrupt_service_routine_18);
    set_idt_entry(0x20, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG, KERNEL_CS, 0, interrupt_service_routine_32);
    set_idt_entry(0x21, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG, KERNEL_CS, 0, interrupt_service_routine_33);
    set_idt_entry(0x22, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG, KERNEL_CS, 0, interrupt_service_routine_34);
    set_idt_entry(0xFF, IDT_PRESENT_FLAG | IDT_INTERRUPT_TYPE_FLAG, KERNEL_CS, 0, interrupt_service_routine_255);


	load_idt();
}

struct cpu_status_t* interrupt_dispatch(struct cpu_status_t* context)
{
    logf("Counter: %d\n", readPITCounter());
    log_interrupt(context);
    // log_heap();
    // kprintf("\nHalted");
    // while(1) asm volatile("hlt");
    logf("Interrupted\n");
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
            while(1) asm("hlt");
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
            timer_handler();
            write_apic_register(APIC_EOI_REGISTER_OFFSET, 0x0l);
        } break;
        case INTR_KEYBOARD_INTERRUPT: {
            handle_keyboard_interrupt();
            write_apic_register(APIC_EOI_REGISTER_OFFSET, 0x0l);
        } break;
        case INTR_PIT_INTERRUPT: {
            pit_irq_handler();
            write_apic_register(APIC_EOI_REGISTER_OFFSET, 0x00l);
        } break;
        case INTR_APIC_SPURIOUS_INTERRUPT: {
            write_apic_register(APIC_EOI_REGISTER_OFFSET, 0x00l);
        } break;
        default: {

        }
    }

    return context;
}