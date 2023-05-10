#include "print.h"
#include "multiboot.h"
#include "utils.h"
#include "Logging.h"
#include "IDT.h"
#include "PMM.h"

extern uint64_t page_table_l4[];

// called inside main64.asm
void kernel_main(struct multiboot_info_header* mboot_header) {
    print_clear();
    print_set_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);
    print_str("Welcome to our 64-bit kernel\n");
	
	init_serial();
	log_mbheader(mboot_header);

	log_page_table(page_table_l4);

	init_idt();
	// pmm_setup(mboot_header);

	/* Uncomment Below Lines to generate dummy Interrupt */
	// char* val = (char*)0x200000;
	// print_str("\nMemory Byte: ");
	// print_num((uint32_t)*val);
}
