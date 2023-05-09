#include "Logging.h"



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

void log_astr(char* string) {
    block_align();
    log_str(string);
}
void log_achar(char ch) {
    block_align();
    log_char(ch);
}
void log_anum(unsigned int num) {
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

void log_str(char *string) {
    while(*string) {
        log_char(*string++);
    }
}

void log_num(unsigned int num) {
    unsigned int n0 = num, n1=0;
    unsigned int dig = 0;

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

void log_tag(struct multiboot_tag* tag) 
{
    const char* multiboot_tag_type[22] = {
        "MULTIBOOT_TAG_TYPE_END",
        "MULTIBOOT_TAG_TYPE_CMDLINE",
        "MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME",
        "MULTIBOOT_TAG_TYPE_MODULE",
        "MULTIBOOT_TAG_TYPE_BASIC_MEMINFO",
        "MULTIBOOT_TAG_TYPE_BOOTDEV",
        "MULTIBOOT_TAG_TYPE_MMAP",
        "MULTIBOOT_TAG_TYPE_VBE",
        "MULTIBOOT_TAG_TYPE_FRAMEBUFFER",
        "MULTIBOOT_TAG_TYPE_ELF_SECTIONS"
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

    const char* multiboot_mmap_entry_type[6] = {
        "",
        "MULTIBOOT_MEMORY_AVAILABLE",              // 1
        "MULTIBOOT_MEMORY_RESERVED",               // 2
        "MULTIBOOT_MEMORY_ACPI_RECLAIMABLE",       // 3
        "MULTIBOOT_MEMORY_NVS",                    // 4
        "MULTIBOOT_MEMORY_BADRAM"                  // 5
    };
    
    const char* multiboot_framebuffer_type[3] = {
        "MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED",
        "MULTIBOOT_FRAMEBUFFER_TYPE_RGB",
        "MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT",
    };

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
        case MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR: {

        } break;
        default: {

        }
    }

    log_tag_end:
    block_end();
}

void log_mbheader(struct multiboot_info_header* mboot_header) {
    struct multiboot_tag* tag = (struct multiboot_tag*)((multiboot_uint8_t*)mboot_header + sizeof(struct multiboot_info_header));

    log_astr("\n\nMULTIBOOT_INFO_HEADER");
    log_endl();
    block_start();
    while(tag && tag->type != MULTIBOOT_TAG_TYPE_END) {
        log_tag(tag);
        tag = (struct multiboot_tag*)((multiboot_uint8_t*)tag + ((tag->size + 7) & ~7));
    }
    block_end();
} 