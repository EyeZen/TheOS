#ifndef _KTIMER_H
#define _KTIMER_H

#include <stdint.h>
#include <stdbool.h>

/* Programmable Interval Timer  */
#define PIT_CHANNEL_0_DATA_PORT 0x40
#define PIT_MODE_COMMAND_REGISTER   0x43

#define IO_APIC_IRQ_TIMER_INDEX 0x14 //Double check

#define PIT_COUNTER_VALUE 0x4A9

#define PIT_CONFIGURATION_BYTE 0b00110100

#define CALIBRATION_MS_TO_WAIT  30

#define APIC_TIMER_SET_PERIODIC 0x20000
#define APIC_TIMER_SET_MASKED   0x10000

/* APIC */
#define APIC_TIMER_LVT_OFFSET 0x00000320

#define APIC_TIMER_IDT_ENTRY 0x20
#define APIC_TIMER_CONFIGURATION_OFFSET 0x3E0
#define APIC_TIMER_INITIAL_COUNT_REGISTER_OFFSET 0x380
#define APIC_TIMER_CURRENT_COUNT_REGISTER_OFFSET 0x390

#define APIC_TIMER_DIVIDER_1 0xB
#define APIC_TIMER_DIVIDER_2 0x0
#define APIC_TIMER_DIVIDER_4 0x1
#define APIC_TIMER_DIVIDER_8 0x2
#define APIC_TIMER_DIVIDER_16 0x3
#define APIC_TIMER_DIVIDER_32 0x8
#define APIC_TIMER_DIVIDER_64 0x9
#define APIC_TIMER_DIVIDER_128 0xA

#define APIC_TIMER_MODE_ONE_SHOT 0x0
#define APIC_TIMER_MODE_PERIODIC 0x20000
#define APIC_VERSION_REGISTER_OFFSET 0x30
#define APIC_EOI_REGISTER_OFFSET 0xB0

uint16_t readPITCounter();

uint32_t calibrate_apic();

void pit_irq_handler();
void timer_handler();
void start_apic_timer(uint32_t, uint32_t, uint8_t divider);

#endif