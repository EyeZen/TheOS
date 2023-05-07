#include "print.h"
#include "multiboot.h"
#include "utils.h"
#include "Logging.h"

#include <stdint.h>


// called inside main64.asm
void kernel_main(struct multiboot_info_header* mboot_header) {
    print_clear();
    print_set_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);
    print_str("Welcome to our 64-bit kernel\n");
	
	init_serial();
	log_mbheader(mboot_header);
}
