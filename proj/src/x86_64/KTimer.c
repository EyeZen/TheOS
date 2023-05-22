#include <KTimer.h>
#include <ACPI.h>
#include <Logging.h>

#define PIT_WAIT() \
        do { \
            asm volatile(\
                "jmp 1f\n\t" \
                "1:\n\t"\
                "jmp 2f\n\t"\
                "2:"\
            );\
        } while(0)

uint8_t pit_timer_counter = 0;
volatile uint32_t pitTicks = 0;
uint32_t apic_calibrated_ticks;

volatile uint64_t scheduler_ticks = 0;

inline unsigned char in(int portnum)
{
    unsigned char data=0;
    __asm__ __volatile__ ("inb %%dx, %%al" : "=a" (data) : "d" (portnum));       
    return data;
}

inline void out(int portnum, unsigned char data)
{
    __asm__ __volatile__ ("outb %%al, %%dx" :: "a" (data),"d" (portnum));        
}


uint16_t readPITCounter() {
    uint16_t counter = in(PIT_CHANNEL_0_DATA_PORT);
    return counter;
}

uint32_t calibrate_apic() {
    //Configuration byte value is: 0b00'11'010'0 where: bits 6-7 are the channel (0), 4-5 the access mode (read/load lsb first then msb)
    // 1-3 mode of operation (we are using mode 2 rate generator), 0: BCD/Binary mode (0 we are using 16bit binary mode) 
    pitTicks = 0;
    out(PIT_MODE_COMMAND_REGISTER, 0b00110100); PIT_WAIT();

    //Sending the counter value to the channel 0 data port
    //The value is 1139 (~1ms) obtained from PIT Clock rate (1193180) / 1000 
    out(PIT_CHANNEL_0_DATA_PORT, PIT_COUNTER_VALUE & 0xFF); PIT_WAIT();
    out(PIT_CHANNEL_0_DATA_PORT, (PIT_COUNTER_VALUE >> 8)); PIT_WAIT();
    //First let's make sure that the APIC timer is stopped, this is achieved by writing 0 to the initial count register.
    write_apic_register(APIC_TIMER_INITIAL_COUNT_REGISTER_OFFSET, 0); PIT_WAIT();
    //Let's set the timer divider to 2
    write_apic_register(APIC_TIMER_CONFIGURATION_OFFSET, APIC_TIMER_DIVIDER_2); PIT_WAIT();
    
    //It's time to get the timers start... followed immediately by the apic..
    set_irq_mask(IOREDTBL2, false); PIT_WAIT();
    //Let's set the APIC Timer initial value to the maximum available
    // Initial value of the apic is the maximum number that can be stored
    write_apic_register(APIC_TIMER_INITIAL_COUNT_REGISTER_OFFSET, (uint32_t)-1); PIT_WAIT();
    //Now it's time to enable interrupts...
    write_apic_register(APIC_TIMER_CONFIGURATION_OFFSET, APIC_TIMER_DIVIDER_2); PIT_WAIT();
    write_apic_register(APIC_TIMER_LVT_OFFSET, APIC_TIMER_SET_PERIODIC | 0x22); PIT_WAIT();
    asm("sti");
    logf("Starting Calibration\n");
    //Now we need to decide how many milliseconds  we want the pit irq to be fired...
    while(pitTicks < CALIBRATION_MS_TO_WAIT);   
    logf("Completed Calibration\n");
    // We waited enough... let's read the apic counter...
    uint32_t current_apic_count = read_apic_register(APIC_TIMER_CURRENT_COUNT_REGISTER_OFFSET);
    // Disable the irqs first
    // Writing 0 to the inital counter register actually disable the timer IRQ
    write_apic_register(APIC_TIMER_INITIAL_COUNT_REGISTER_OFFSET, 0);
    set_irq_mask(IOREDTBL2, true);
    // Let´s do some math...
    // First: we need to know how many ticks were done in the apic
    // the current count register is a countdown, so we need basically to do: INITIAL_COUNT - CURRENT_COUNT
    uint32_t time_elapsed = ((uint32_t)-1) - current_apic_count;
    // now we want to know how many apic ticks are in 1ms so we divide per CALIBRATION_MS_TO_WAIT 
    apic_calibrated_ticks = time_elapsed / CALIBRATION_MS_TO_WAIT;
    //Let's store the result along with the divider in the kernel_settings.
    // kernel_settings.apic_timer.timer_ticks_base = apic_calibrated_ticks;
    // kernel_settings.apic_timer.timer_divisor = APIC_TIMER_DIVIDER_2;
    // et voila... calibration done, now we can use this value as a base for the initial count register
    return apic_calibrated_ticks;
}

void start_apic_timer(uint32_t initial_count, uint32_t flags, uint8_t divider) {

    if(apic_base_address == 0) {
        logf("Apic_base_address not found, or apic not initialized\n");
    }

    logf("Read apic_register: 0x%x\n", read_apic_register(APIC_TIMER_LVT_OFFSET));

    write_apic_register(APIC_TIMER_INITIAL_COUNT_REGISTER_OFFSET, initial_count);
    write_apic_register(APIC_TIMER_CONFIGURATION_OFFSET, divider);
    write_apic_register(APIC_TIMER_LVT_OFFSET, flags | APIC_TIMER_IDT_ENTRY);
    asm("sti");
}

void timer_handler() {
    scheduler_ticks++;
    // logf("\nsched ticks: %d\n", scheduler_ticks);
// #if USE_FRAMEBUFFER == 1
    if(pit_timer_counter == 0) {
        pit_timer_counter = 1;
    } else {
        pit_timer_counter = 0;
    }
// #endif
    // if(scheduler_ticks > 1000) {
    //     write_apic_register(APIC_TIMER_INITIAL_COUNT_REGISTER_OFFSET, 0);
    //     PIT_WAIT();
    //     logf("\nTimer Should Stop\n");
    // }
}



void pit_irq_handler() {
    pitTicks++;
// #if USE_FRAMEBUFFER == 1
    if(pit_timer_counter == 0) {
        pit_timer_counter = 1;
    } else {
        pit_timer_counter = 0;
    }
// #endif
}