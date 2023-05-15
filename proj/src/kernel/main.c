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

const uint64_t end_of_mapped_memory = 4*MiB - 8;
extern uint64_t _kernel_end;
extern char *_binary_proj_res_fonts_default_font_psf_start;

void kernel_main(struct multiboot_info_header* mboot_header) {
	// console_set_type(CONSOLE_TYPE_LOG);

    print_clear();
    print_set_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);
    kprintf("Welcome to our 64-bit kernel\n");
	
	init_serial();

	log_mbheader(mboot_header);

	init_idt();
	pmm_setup(mboot_header);
	kheap_init();

	logfa("System:");
	block_start();
		logfa("total_mem: %ull\n",phys_mem.frame_count * PAGE_SIZE);
		logfa("kernel_end: %ull\n",_kernel_end);
		logfa("end of mapped memory: %ull\n",end_of_mapped_memory);
	block_end();

	struct multiboot_tag_framebuffer* fb_tag = (struct multiboot_tag_framebuffer*)find_tag(mboot_header, MULTIBOOT_TAG_TYPE_FRAMEBUFFER);
	fb_init(fb_tag);
	fb_draw_logo();

	fb_printStr("Hello Framebuffer!", 0, 0, WHITE, BLACK, &rasci_font);
	fb_printStr("\nHow is it?", 0, 0, RED, GREEN, &rasci_font);
}
