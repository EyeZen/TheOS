#include "Logging.h"
#include "utils.h"
#include "IDT.h"
#include "RSDP.h"

#define PAGE_FRAME_ADDRESS(page_frame) (page_frame & 0x000FFFFFFFFFF000)


static const char *exception_names[] = {
  "Divide by Zero Error",
  "Debug",
  "Non Maskable Interrupt",
  "Breakpoint",
  "Overflow",
  "Bound Range",
  "Invalid Opcode",
  "Device Not Available",
  "Double Fault",
  "Coprocessor Segment Overrun",
  "Invalid TSS",
  "Segment Not Present",
  "Stack-Segment Fault",
  "General Protection Fault",
  "Page Fault",
  "Reserved",
  "x87 Floating-Point Exception",
  "Alignment Check",
  "Machine Check",
  "SIMD Floating-Point Exception",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Security Exception",
  "Reserved"
};

static const char* multiboot_tag_type[22] = {
    "MULTIBOOT_TAG_TYPE_END",
    "MULTIBOOT_TAG_TYPE_CMDLINE",
    "MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME",
    "MULTIBOOT_TAG_TYPE_MODULE",
    "MULTIBOOT_TAG_TYPE_BASIC_MEMINFO",
    "MULTIBOOT_TAG_TYPE_BOOTDEV",
    "MULTIBOOT_TAG_TYPE_MMAP",
    "MULTIBOOT_TAG_TYPE_VBE",
    "MULTIBOOT_TAG_TYPE_FRAMEBUFFER",
    "MULTIBOOT_TAG_TYPE_ELF_SECTIONS",
    "MULTIBOOT_TAG_TYPE_APM",
    "MULTIBOOT_TAG_TYPE_EFI32",
    "MULTIBOOT_TAG_TYPE_EFI64",
    "MULTIBOOT_TAG_TYPE_SMBIOS",
    "MULTIBOOT_TAG_TYPE_ACPI_OLD",
    "MULTIBOOT_TAG_TYPE_ACPI_NEW",
    "MULTIBOOT_TAG_TYPE_NETWORK",
    "MULTIBOOT_TAG_TYPE_EFI_MMAP",
    "MULTIBOOT_TAG_TYPE_EFI_BS",
    "MULTIBOOT_TAG_TYPE_EFI32_IH",
    "MULTIBOOT_TAG_TYPE_EFI64_IH",
    "MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR",
};

static const char* multiboot_mmap_entry_type[6] = {
    "",
    "MULTIBOOT_MEMORY_AVAILABLE",              // 1
    "MULTIBOOT_MEMORY_RESERVED",               // 2
    "MULTIBOOT_MEMORY_ACPI_RECLAIMABLE",       // 3
    "MULTIBOOT_MEMORY_NVS",                    // 4
    "MULTIBOOT_MEMORY_BADRAM"                  // 5
};

static const char* multiboot_framebuffer_type[3] = {
    "MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED",
    "MULTIBOOT_FRAMEBUFFER_TYPE_RGB",
    "MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT",
};


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


int _block_level_count = 0;

void reset_block() { _block_level_count = 0; }
void block_align() {
    for(int blk = 0; blk < _block_level_count; blk++) {
        log_char('\t');
    }
}

void log_astr(const char* string) {
    block_align();
    log_str(string);
}
void log_achar(char ch) {
    block_align();
    log_char(ch);
}
void log_anum(uint64_t num) {
    block_align();
    log_num(num);
}

void block_start() {
    log_astr("{\n"); 
    _block_level_count++; 
}
void block_end() { 
    _block_level_count--; 
    // log_char('\n');
    log_astr("}\n");
}



int init_serial() {
    out(PORT + 1, 0x00);    // Disable all interrupts
    out(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    out(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    out(PORT + 1, 0x00);    //                  (hi byte)
    out(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    out(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold   
    out(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
    out(PORT + 4, 0x1E);    // Set in loopback mode, test the serial chip        
    out(PORT + 0, 0xAE);    // Test serial chip (send byte 0xAE and check if serial returns same byte)

    // Check if serial is faulty (i.e: not same byte as sent)
    if(in(PORT + 0) != 0xAE) {
        return 1;
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    out(PORT + 4, 0x0F);
    return 0;
}

void log_char(char ch) {
    out(PORT, ch);
}

void log_str(const char *string) {
    while(*string) {
        log_char(*string++);
    }
}

void log_num(uint64_t num) {
    uint64_t n0 = num, n1=0;
    uint64_t dig = 0;

    if(num == 0) {
        out(PORT, '0');
        return;
    }

    while(n0) {
        n1 = n1 * 10 + (n0 % 10);
        n0 /= 10;
        dig++;
    }
    while(dig--) {
        n0 = n1 % 10;
        n1 /= 10;
        out(PORT, (char)('0' + n0));
    }
}

void log_endl() { log_char('\n'); }


void log_page_table(uint64_t* pml4t) 
{
	log_astr("Page Table\n");
	log_astr("PML4T:"); log_num((uint64_t)pml4t); log_endl();
	block_start();
	// Level 4: PML4T Traversal
	for(int itr_pml4 = 0; itr_pml4 < 512; itr_pml4++) {
		if(pml4t[itr_pml4] & 0x1ull) {
            uint64_t* pdprt = (uint64_t*)PAGE_FRAME_ADDRESS(pml4t[itr_pml4]); // bits 51 : 12 

            if(pdprt == pml4t) {
                log_astr("PDPR_");log_num(itr_pml4);log_str(": ");
                log_str("[PML4T]");
                log_endl();
                continue;
            }

			log_astr("PDPR_");log_num(itr_pml4);log_str(": ");
			log_num(pml4t[itr_pml4]);log_char(':'); log_num((uint64_t)pdprt);
            log_endl();

            block_start();
            // Level 3: PDPT Traversal
            for(int itr_pdpr = 0; itr_pdpr < 512; itr_pdpr++) {
                if(pdprt[itr_pdpr] & 0x1ull) {
                    uint64_t* pdt = (uint64_t*)PAGE_FRAME_ADDRESS(pdprt[itr_pdpr]);

                    if(pdt == pdprt) {
                        log_astr("PD_");log_num(itr_pdpr);log_str(": ");
                        log_str("[PDPR_");log_num(itr_pml4);log_char(']');
                        log_endl();
                        continue;
                    }

                    log_astr("PD_");log_num(itr_pdpr);log_str(": ");
                    log_num(pdprt[itr_pdpr]);log_char(':'); log_num((uint64_t)pdt);
                    log_endl();

                    block_start();
                    // Level 2: PDT Traversal
                    for(int itr_pdt = 0; itr_pdt < 512; itr_pdt++) {
                        if(pdt[itr_pdt] & 0x1ull) {
                            uint64_t* pt = (uint64_t*)PAGE_FRAME_ADDRESS(pdt[itr_pdt]);

                            if(pt == pdt) {
                                log_astr("PT_");log_num(itr_pdt);log_str(": ");
                                log_str("[PD_");log_num(itr_pdpr);log_char(']');
                                log_endl();
                                continue;
                            }

                            log_astr("PT_");log_num(itr_pdt);log_str(": ");
                            log_num(pdt[itr_pdt]);log_char(':'); log_num((uint64_t)pt);
                            log_endl();

                            block_start();
                            // Level 1: PT Traversal
                            for(int itr_pt = 0; itr_pt < 512; itr_pt++) {
                                if(pt[itr_pt] & 0x1ull) {
                                    uint64_t* page = (uint64_t*)PAGE_FRAME_ADDRESS(pt[itr_pt]);

                                    if(page == pt) {
                                        log_astr("Page_");log_num(itr_pt);log_str(":");
                                        log_str("[PT_");log_num(itr_pdt);log_char(']');
                                        log_endl();
                                        continue;
                                    }

                                    log_astr("Page_");log_num(itr_pt);log_str(":");
                                    log_num(pt[itr_pt]);log_char(':'); log_num((uint64_t)page);
                                    log_endl();
                                }
                            }
                            block_end();

                        }
                    }
                    block_end();
                }
            }
            block_end();
		}
	}
	block_end();
}

void log_tag(struct multiboot_tag* tag) 
{

    // reset_block();

    log_astr((char*)multiboot_tag_type[tag->type]);
    log_endl();

    block_start();
    log_astr("type: "); log_num(tag->type); log_endl();
    log_astr("size: "); log_num(tag->size); log_endl();
    switch(tag->type) {
        case MULTIBOOT_TAG_TYPE_END: {
            goto log_tag_end;
        } break;
        case MULTIBOOT_TAG_TYPE_CMDLINE: {
            struct multiboot_tag_string* t_cmdline = (struct multiboot_tag_string*)tag;
            log_astr("string: ");
            // log_char(t_cmdline->string[0]);
            log_str(t_cmdline->string); log_endl();
        } break;
        case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME: {
            struct multiboot_tag_string* t_loader = (struct multiboot_tag_string*)tag;
            log_astr("string: ");
            // log_char(t_loader->string[0]);
            log_str(t_loader->string);                          log_endl();
        } break;
        case MULTIBOOT_TAG_TYPE_MODULE: {
            struct multiboot_tag_module* t_mod = (struct multiboot_tag_module*)tag;
            log_astr("mod_start: "); log_num(t_mod->mod_start); log_endl();
            log_astr("mod_end: "); log_num(t_mod->mod_end);     log_endl();
            log_astr("cmdline: "); log_char(t_mod->cmdline[0]); log_endl();
        } break;
        case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO: {
            struct multiboot_tag_basic_meminfo* t_bmi = (struct multiboot_tag_basic_meminfo*)tag;
            log_astr("mem_lower: "); log_num(t_bmi->mem_lower); log_endl();
            log_astr("mem_upper: "); log_num(t_bmi->mem_upper); log_endl();
        } break;
        case MULTIBOOT_TAG_TYPE_BOOTDEV: {
            struct multiboot_tag_bootdev* t_bdev = (struct multiboot_tag_bootdev*)tag;
            log_astr("biosdev: "); log_num(t_bdev->biosdev);     log_endl();
            log_astr("part: "); log_num(t_bdev->part);            log_endl();
            log_astr("slice: "); log_num(t_bdev->slice);          log_endl();

        } break;
        case MULTIBOOT_TAG_TYPE_MMAP: {
            struct multiboot_tag_mmap* t_mmap = (struct multiboot_tag_mmap*)tag; 
            log_astr("entry_size: "); log_num(t_mmap->entry_size);          log_endl();
            log_astr("entry_version: "); log_num(t_mmap->entry_version);    log_endl();

            struct multiboot_mmap_entry* entry = t_mmap->entries;
            for(
                entry = t_mmap->entries;
                (unsigned char*)entry < (unsigned char*)t_mmap + t_mmap->size;   
                entry = (struct multiboot_mmap_entry*)((unsigned char*)entry + t_mmap->entry_size)
                )
            {
                log_endl();
                log_astr("Entry:");
                block_start();
                    log_astr("base_addr: "); log_num(entry->addr);          log_endl();
                    log_astr("length: "); log_num(entry->len);              log_endl();
                    log_astr("type: "); log_str((char*)multiboot_mmap_entry_type[entry->type]);     
                    log_endl();
                block_end();
            }
        } break;
        case MULTIBOOT_TAG_TYPE_VBE: {

        } break;
        case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
            struct multiboot_tag_framebuffer* t_fb = (struct multiboot_tag_framebuffer*)tag;
            log_astr("address: ");      log_num(t_fb->common.framebuffer_addr);     log_endl();
            log_astr("width: ");        log_num(t_fb->common.framebuffer_width);    log_endl();
            log_astr("height: ");       log_num(t_fb->common.framebuffer_height);   log_endl();
            log_astr("bpp: ");          log_num(t_fb->common.framebuffer_bpp);      log_endl();
            log_astr("pitch: ");        log_num(t_fb->common.framebuffer_pitch);    log_endl();
            log_astr("buffer-type: ");  log_str((char*)multiboot_framebuffer_type[t_fb->common.framebuffer_type]);
            log_endl();
        } break;
        case MULTIBOOT_TAG_TYPE_ACPI_OLD: {
            struct multiboot_tag_old_acpi* t_nacip = (struct multiboot_tag_old_acpi*)tag;
            struct RSDPDescriptor* rsdp_desc = (struct RSDPDescriptor*)t_nacip->rsdp; 

            log_astr("RSDPDescriptor:\n");
            block_start();
                log_astr("Signature: ");
                    for(int i=0; i<8; i++) log_char(rsdp_desc->Signature[i]);
                log_endl();
                log_astr("Checksum: "); log_num(rsdp_desc->Checksum); log_endl();
                log_astr("OEMID: ");
                    for(int i=0; i<6; i++) log_char(rsdp_desc->OEMID[i]);
                log_endl();
                log_astr("Revision: "); log_num(rsdp_desc->Revision); log_endl();
                log_astr("RSDTAddress: "); log_num(rsdp_desc->RSDTAddress); log_endl();
            block_end();

        } break;
        default: {

        }
    }

    log_tag_end:
    block_end();
}

void log_mbheader(struct multiboot_info_header* mboot_header) {
    struct multiboot_tag* tag = (struct multiboot_tag*)((multiboot_uint8_t*)mboot_header + sizeof(struct multiboot_info_header));

    log_astr("\n\nMULTIBOOT_INFO_HEADER: "); log_num((uint64_t)mboot_header);
    log_endl();
    block_start();
    while(tag && tag->type != MULTIBOOT_TAG_TYPE_END) {
        log_tag(tag);
        tag = (struct multiboot_tag*)((multiboot_uint8_t*)tag + ((tag->size + 7) & ~7));
    }
    block_end();
} 

void log_interrupt(struct cpu_status_t* context)
{
    log_astr("Interrupt: "); log_astr(exception_names[context->vector_number]);
    log_endl();

    block_start();
    log_astr("Vector#: "); log_num(context->vector_number); log_endl();
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
        default: 
            log_astr("Undefined Interrupt");
    }
    block_end();
}