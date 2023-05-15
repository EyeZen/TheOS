#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#define INTR_DIVIDE_ERROR 0
#define INTR_DEBUG_EXC 1
#define INTR_NMI_INTERRUPT 2
#define INTR_BREAKPOINT 3
#define INTR_OVERFLOW 4
#define INTR_BOUND_RANGE_EXCEED 5
#define INTR_INVALID_OPCODE 6
#define INTR_DEV_NOT_AVL 7
#define INTR_DOUBLE_FAULT 8
#define INTR_COPROC_SEG_OVERRUN 9
#define INTR_INVALID_TSS 10
#define INTR_SEGMENT_NOT_PRESENT 11
#define INTR_STACK_SEGMENT_FAULT 12
#define INTR_GENERAL_PROTECTION 13
#define INTR_PAGE_FAULT 14
#define INTR_INT_RSV 15
#define INTR_FLOATING_POINT_ERR 16
#define INTR_ALIGNMENT_CHECK 17
#define INTR_MACHINE_CHECK 18
#define INTR_SIMD_FP_EXC 19
#define INTR_APIC_TIMER_INTERRUPT 32
#define INTR_KEYBOARD_INTERRUPT 33
#define INTR_PIT_INTERRUPT 34
#define INTR_APIC_SPURIOUS_INTERRUPT 255


#define PF_PRESENT              (1 << 0)
#define PF_WRITE                (1 << 1)
#define PF_USER                 (1 << 2)
#define PF_INSTRUCTION_FETCH    (1 << 4)


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

struct idtr
{
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

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


void load_idt();

void init_idt();

void interrupt_dispatch(struct cpu_status_t* context);

void set_idt_entry(uint8_t vector, void(*handler)(void), uint8_t dpl);

#endif