#include "print.h"
#include "multiboot.h"
#include "utils.h"
#include "Logging.h"
#include "IDT.h"
#include "PMM.h"
#include "MEM.h"
#include "VMM.h"

// called inside main64.asm
void kernel_main(struct multiboot_info_header* mboot_header) {
    print_clear();
    print_set_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);
    print_str("Welcome to our 64-bit kernel\n");
	
	init_serial();

	log_mbheader(mboot_header);

	init_idt();

	pmm_setup(mboot_header);

	char* some_far_away_addr = (char*)0x800000;

	some_far_away_addr = (char*)map_vaddress(some_far_away_addr, 0);

	*some_far_away_addr = 5;

	log_page_table((uint64_t*)(SIGN_EXTENSION | ENTRIES_TO_ADDRESS(511L, 511L, 511L, 511L)));
}
