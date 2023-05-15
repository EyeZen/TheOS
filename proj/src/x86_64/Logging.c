#include "Logging.h"
#include "utils.h"
#include "IDT.h"
#include "RSDP.h"
#include "KHeap.h"
#include "Fonts.h"


// #define PORT 0x3f8

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


int _block_level_count = 0;

void reset_block() { _block_level_count = 0; }
void block_align() {

    for(int blk = 0; blk < _block_level_count; blk++) {
        logf("\t");
    }
}

void block_start() {
    logfa("{\n"); 
    _block_level_count++; 
}
void block_end() { 
    _block_level_count--; 
    // log_char('\n');
    logfa("}\n");
}

///////////////////////////////////

void log_page_table(uint64_t* pml4t) 
{
    logfa("Page Table\n");
    logfa("PML4T: %ull\n", (uint64_t)pml4t);

	block_start();
	// Level 4: PML4T Traversal
	for(int itr_pml4 = 0; itr_pml4 < 512; itr_pml4++) {
		if(pml4t[itr_pml4] & 0x1ull) {
            uint64_t* pdprt = (uint64_t*)PAGE_FRAME_ADDRESS(pml4t[itr_pml4]); // bits 51 : 12 

            if(pdprt == pml4t) {
                logfa("PDPR_%d: [PML4T]\n", itr_pml4);
                continue;
            }
            logfa("PDPR_%d: %d: %ull\n", itr_pml4, pml4t[itr_pml4], (uint64_t)pdprt)

            block_start();
            // Level 3: PDPT Traversal
            for(int itr_pdpr = 0; itr_pdpr < 512; itr_pdpr++) {
                if(pdprt[itr_pdpr] & 0x1ull) {
                    uint64_t* pdt = (uint64_t*)PAGE_FRAME_ADDRESS(pdprt[itr_pdpr]);

                    if(pdt == pdprt) {
                        logfa("PD_%d: [PDPR_%d]\n", itr_pdpr, itr_pml4);
                        continue;
                    }

                    logfa("PD_%d: %d: %ull\n", itr_pdpr, pdprt[itr_pdpr], (uint64_t)pdt);

                    block_start();
                    // Level 2: PDT Traversal
                    for(int itr_pdt = 0; itr_pdt < 512; itr_pdt++) {
                        if(pdt[itr_pdt] & 0x1ull) {
                            uint64_t* pt = (uint64_t*)PAGE_FRAME_ADDRESS(pdt[itr_pdt]);

                            if(pt == pdt) {
                                logfa("PT_%d: [PD_%d]\n", itr_pdt, itr_pdpr);\
                                continue;
                            }

                            logfa("PT_%d: %d:%ull\n", itr_pdt, pdt[itr_pdt], (uint64_t)pt);

                            block_start();
                            // Level 1: PT Traversal
                            for(int itr_pt = 0; itr_pt < 512; itr_pt++) {
                                if(pt[itr_pt] & 0x1ull) {
                                    uint64_t* page = (uint64_t*)PAGE_FRAME_ADDRESS(pt[itr_pt]);

                                    if(page == pt) {
                                        logfa("Page_%d:[PT_%d]\n", itr_pt, itr_pdt);
                                        continue;
                                    }
                                    logfa("Page_%d:%d:%ull\n", itr_pt, pt[itr_pt], (uint64_t)page);
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
    logfa("%s\n", multiboot_tag_type[tag->type]);
    block_start();
    logfa("type: %d\n", tag->type);
    logfa("size: %d\n", tag->size);
    switch(tag->type) {
        case MULTIBOOT_TAG_TYPE_END: {
            goto log_tag_end;
        } break;
        case MULTIBOOT_TAG_TYPE_CMDLINE: {
            struct multiboot_tag_string* t_cmdline = (struct multiboot_tag_string*)tag;
            logfa("string: %s\n", t_cmdline->string);
        } break;
        case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME: {
            struct multiboot_tag_string* t_loader = (struct multiboot_tag_string*)tag;
            logfa("string: %s\n", t_loader->string);
        } break;
        case MULTIBOOT_TAG_TYPE_MODULE: {
            struct multiboot_tag_module* t_mod = (struct multiboot_tag_module*)tag;
            logfa("mod_start: %d\n", t_mod->mod_start); 
            logfa("mod_end: %d\n", t_mod->mod_end); 
            logfa("cmdline: %d\n", t_mod->cmdline[0]);
        } break;
        case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO: {
            struct multiboot_tag_basic_meminfo* t_bmi = (struct multiboot_tag_basic_meminfo*)tag;
            logfa("mem_lower: %d\n", t_bmi->mem_lower);
            logfa("mem_upper: %d\n", t_bmi->mem_upper);
        } break;
        case MULTIBOOT_TAG_TYPE_BOOTDEV: {
            struct multiboot_tag_bootdev* t_bdev = (struct multiboot_tag_bootdev*)tag;
            logfa("biosdev: \n", t_bdev->biosdev);
            logfa("part: \n"   , t_bdev->part);
            logfa("slice: \n"  , t_bdev->slice);

        } break;
        case MULTIBOOT_TAG_TYPE_MMAP: {
            struct multiboot_tag_mmap* t_mmap = (struct multiboot_tag_mmap*)tag; 
            logfa("entry_size: %d\n", t_mmap->entry_size);
            logfa("entry_version: %d\n", t_mmap->entry_version);

            struct multiboot_mmap_entry* entry = t_mmap->entries;
            for(
                entry = t_mmap->entries;
                (unsigned char*)entry < (unsigned char*)t_mmap + t_mmap->size;   
                entry = (struct multiboot_mmap_entry*)((unsigned char*)entry + t_mmap->entry_size)
                )
            {
                logf("\n");
                logfa("Entry:");
                block_start();
                    logfa("base_addr: %d\n", entry->addr);
                    logfa("length: %d\n", entry->len);
                    logfa("type: %s\n", multiboot_mmap_entry_type[entry->type]);
                block_end();
            }
        } break;
        case MULTIBOOT_TAG_TYPE_VBE: {

        } break;
        case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
            struct multiboot_tag_framebuffer* t_fb = (struct multiboot_tag_framebuffer*)tag;
            logfa("address: %d\n", t_fb->common.framebuffer_addr);
            logfa("width: %d\n", t_fb->common.framebuffer_width);
            logfa("height: %d\n", t_fb->common.framebuffer_height);
            logfa("bpp: %d\n", t_fb->common.framebuffer_bpp);
            logfa("pitch: %d\n", t_fb->common.framebuffer_pitch);
            logfa("buffer-type: %s\n", multiboot_framebuffer_type[t_fb->common.framebuffer_type]);
        } break;
        case MULTIBOOT_TAG_TYPE_ACPI_OLD: {
            struct multiboot_tag_old_acpi* t_nacip = (struct multiboot_tag_old_acpi*)tag;
            struct RSDPDescriptor* rsdp_desc = (struct RSDPDescriptor*)t_nacip->rsdp; 

            logfa("RSDPDescriptor:\n");
            block_start();
                logfa("Signature: ");
                    for(int i=0; i<8; i++) logf("%c", rsdp_desc->Signature[i]);
                logf("\n");
                logfa("Checksum: %d\n", rsdp_desc->Checksum);
                logfa("OEMID: ");
                    for(int i=0; i<6; i++) logf("%c", rsdp_desc->OEMID[i]);
                logf("\n");
                logfa("Revision: %d\n", rsdp_desc->Revision);
                logfa("RSDTAddress: %d\n", rsdp_desc->RSDTAddress);
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

    logfa("\n\nMULTIBOOT_INFO_HEADER: %ull\n", (uint64_t)mboot_header);
    block_start();
    while(tag && tag->type != MULTIBOOT_TAG_TYPE_END) {
        log_tag(tag);
        tag = (struct multiboot_tag*)((multiboot_uint8_t*)tag + ((tag->size + 7) & ~7));
    }
    block_end();
} 

void log_context(struct cpu_status_t* context) {
    logfa("CPU-Context:\n");
    block_start();
        logfa("r15: %ull\n",        (uint64_t)context->r15);
        logfa("r14: %ull\n",        (uint64_t)context->r14);
        logfa("r13: %ull\n",        (uint64_t)context->r13);
        logfa("r12: %ull\n",        (uint64_t)context->r12);
        logfa("r11: %ull\n",        (uint64_t)context->r11);
        logfa("r10: %ull\n",        (uint64_t)context->r10);
        logfa("r9: %ull\n",         (uint64_t)context->r9);
        logfa("r8: %ull\n",         (uint64_t)context->r8);
        logfa("rbp: %ull\n",        (uint64_t)context->rbp);
        logfa("rdi: %ull\n",        (uint64_t)context->rdi);
        logfa("rsi: %ull\n",        (uint64_t)context->rsi);
        logfa("rdx: %ull\n",        (uint64_t)context->rdx);
        logfa("rcx: %ull\n",        (uint64_t)context->rcx);
        logfa("rbx: %ull\n",        (uint64_t)context->rbx);
        logfa("rax: %ull\n",        (uint64_t)context->rax);
        logfa("iret_rip: %ull\n",   (uint64_t)context->iret_rip);
        logfa("iret_cs: %ull\n",    (uint64_t)context->iret_cs);
        logfa("iret_flags: %ull\n", (uint64_t)context->iret_flags);
        logfa("iret_rsp: %ull\n",   (uint64_t)context->iret_rsp);
        logfa("iret_ss: %ull\n",    (uint64_t)context->iret_ss);
    block_end();
}

void log_interrupt(struct cpu_status_t* context)
{
    logfa("Interrupt: %s\n", exception_names[context->vector_number]);
    block_start();
    logfa("Vector#: %d\n", context->vector_number);
    log_context(context);
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
            logfa("PF_ERROR_CODE:");
            block_start();
                if(context->error_code & PF_PRESENT) logfa("PRESENT ");
                if(context->error_code & PF_WRITE) logfa("WRITE");
                if(context->error_code & PF_INSTRUCTION_FETCH) logfa("INSPF_INSTRUCTION_FETCH");
                if(context->error_code & PF_USER) logfa("USPF_USER");
                logf("\n");
            block_end();

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
            logfa("Undefined Interrupt");
    }
    block_end();
}

void log_heap() {
    struct KHeapNode* node = kheap_start;
    int node_idx=0;

    logfa("KHeap:\n");
    block_start();
    while(node != NULL) {
        logfa("node_%d[%ull]\n", node_idx, (uint64_t)node); 
        block_start();
            if(node == kheap_start) { logfa("*heap-start\n"); }
            else {
                logfa("heap-start: %ull\n", (uint64_t)kheap_start);
            }

            if(node == kheap_end) { logfa("*heap-end\n"); }
            else {
                logfa("heap-end: %ull\n", (uint64_t)kheap_end);
            }
            if(node == kheap_curr) logfa("*current\n");

            logfa("size: %d\n", node->size);
            logfa("free: ");
            if(node->free) { logf("true\n"); }
            else logf("false\n");

            logfa("prev: ");
            if(node->prev == NULL){ logf("NULL\n"); }
            else logf("%ull\n", (uint64_t)(node->prev));

            logfa("next: ");
            if(node->next == NULL){ logf("NULL\n"); }
            else logf("%ull\n", (uint64_t)(node->next));
        block_end();

        node = node->next;
        node_idx++;
    }
    block_end();
}

void log_glyph(char symbol, Font* font) {
    uint8_t* glyph = (uint8_t*)get_glyph(symbol, font);
    uint32_t row_bytes = (font->glyph_width + 7) / 8;
    logf("%c\n", symbol);
    block_start();
    for(uint32_t y=0; y<font->glyph_height; y++) {
        logf(" ");
        for(uint32_t x=0; x < font->glyph_width; x++) {
            if(glyph[x/8] & (0x80 >> (x & 7))) {
                logf("1");
            } else {
                logf("0");
            }
        }
        logf("\n");
        glyph += row_bytes;
    }
    block_end();
}

void log_font_header(uint64_t font_start_address)
{
    logfa("Font:");
    block_start();

    uint8_t* font_ptr = (uint8_t*)font_start_address;
    if(*(uint16_t*)font_ptr == (uint16_t)PSF1_MAGIC)
    {
        logfa("Version: PSF1\n");
    }
    else if(*(uint32_t*)font_ptr == (uint32_t)PSF2_MAGIC) 
    {
        logfa("Version: PSF2\n");
    }
    else 
    {
        logfa("Version: Unknown\n");
    }
    logfa("Magic: %ul\n", *(uint32_t*)font_ptr);

    block_end();
}
