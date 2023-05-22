#include "print.h"
#include "multiboot.h"
#include "utils.h"
#include "Logging.h"
#include "IDT.h"
#include "PMM.h"
#include "MEM.h"
#include "VMM.h"
#include "KHeap.h"
#include "fbuff.h"

#include "Fonts.h"
#include "RASCI.h"

#include "ACPI.h"
#include <KTimer.h>
#include <Keyboard.h>

const uint64_t end_of_mapped_memory = 4*MiB - 8;
extern uint64_t _kernel_end;
extern char *_binary_proj_res_fonts_default_font_psf_start;

void kernel_main(struct multiboot_info_header* mboot_header) {
	init_serial();
	init_idt();
	pmm_setup(mboot_header);
	kheap_init();
	
	acpi_init(mboot_header);

	struct multiboot_tag_framebuffer* fb_tag = (struct multiboot_tag_framebuffer*)find_tag(mboot_header, MULTIBOOT_TAG_TYPE_FRAMEBUFFER);
	fb_init(fb_tag);
	fb_draw_logo();


	console_set_type(CONSOLE_TYPE_FRAMEBUFFER);
    print_clear();
    // print_set_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);
    print_set_color(WHITE, GREEN);
    kprintf("Welcome to our 64-bit kernel\n\n");

	init_keyboard();

	// uint32_t apic_ticks = calibrate_apic();
	uint32_t apic_ticks = 10000;
	logf("(kernel_main) Calibrated apic value: %u\n", apic_ticks);
	
	log_mbheader(mboot_header);
	logfa("System:");
	block_start();
		logfa("total_mem: %ull\n",phys_mem.frame_count * PAGE_SIZE);
		logfa("kernel_end: %ull\n",_kernel_end);
		logfa("end of mapped memory: %ull\n",end_of_mapped_memory);
	block_end();


	kprintf("Hello There\n");
	start_apic_timer(apic_ticks, APIC_TIMER_SET_PERIODIC, APIC_TIMER_DIVIDER_2);
	// log_page_table((uint64_t)(SIGN_EXTENSION|ENTRIES_TO_ADDRESS(511L, 511L, 511L, 511L)));
	while(1);
}
