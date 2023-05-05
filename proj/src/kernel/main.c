#include "print.h"
#include "multiboot.h"
#include "utils.h"
#include <stdint.h>

struct MB_header {
	uint32_t total_size;
	uint32_t reserved;
} __attribute__((packed));

struct MB_tag_header {
	uint32_t type;
	uint32_t size;
} __attribute__((packed));
struct MB_command_line {
	struct MB_tag_header header;
	char* string;
} __attribute__((packed));

struct MB_tag_mmap_entry {
	uint64_t base_addr;
	uint64_t length;
	uint32_t type;
	uint32_t reserved;
} __attribute__((packed));
struct MB_tag_mmap {
	struct MB_tag_header header;
	uint32_t entry_size;
	uint32_t entry_version;
	struct MP_tag_mmap_entry* entries;
} __attribute__((packed));

struct MB_tag_basic_mem_info {
	struct MB_tag_header header;
	uint32_t mem_lower;
	uint32_t mem_upper;
} __attribute__((packed));

#define ROUNDUP8(addr) ((addr + 7) & ~7)

struct MB_tag_header* find_tag(struct MB_header* mb_info_start, uint32_t type);
struct MB_tag_header* find_next(struct MB_tag_header* current);

// called inside main64.asm
void kernel_main(void* arg1) {
    print_clear();
    print_set_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);
    print_str("Welcome to our 64-bit kernel\n");

	struct MB_header* mboot_header = (struct MB_header*)arg1;
	print_str("mboot_header->total_size: "); print_num(mboot_header->total_size);
	
	struct MB_tag_header* tag = (struct MB_tag_header*)((uint8_t*)mboot_header + sizeof(mboot_header));
	for(int i=0; i < 10; i++) {
		if(tag->type == 0) break;
		print_str("\ntag->type: "); print_num(tag->type);
		tag = (struct MB_tag_header*)((uint8_t*)tag + ((tag->size + 7) & ~7));
	}

	struct MB_tag_basic_mem_info* mem_info = (struct MB_tag_basic_mem_info*)find_tag(mboot_header, 4);

	print_newline();
	print_str("\nmem_info->type: "); print_num(mem_info->header.type);
	print_str("\nmem_info->size: "); print_num(mem_info->header.size);
	print_str("\nmem_info->mem_lower: "); print_num(mem_info->mem_lower);
	print_str("\nmem_info->mem_upper: "); print_num(mem_info->mem_upper);

	tag = find_next((struct MB_tag_header*)mem_info);
	print_str("\ntag->type: "); print_num(tag->type);
	print_str("\ntag->size: "); print_num(tag->size);
	
	tag = find_next(tag);
	print_str("\ntag->type: "); print_num(tag->type);
	print_str("\ntag->size: "); print_num(tag->size);
	
	tag = find_next(tag);
	print_str("\ntag->type: "); print_num(tag->type);
	print_str("\ntag->size: "); print_num(tag->size);



	// struct MB_tag_header* tag_header = (struct MB_tag_header*)((uint8_t*)mboot_header + sizeof(mboot_header));
	// print_str("\ntag_header->type: "); print_num(tag_header->type);
	// print_str("\ntag_header->size: "); print_num(tag_header->size);

	// print_newline();
	// tag_header = (struct MB_tag_header*)((uint8_t*)tag_header + ((tag_header->size + 7) & ~7));
	// print_str("\ntag_header->type: "); print_num(tag_header->type);
	// print_str("\ntag_header->size: "); print_num(tag_header->size);
	// if(tag_header->type == 1) {
	// 	struct MB_command_line* cmdline = (struct MB_command_line*)tag_header;
	// 	uint32_t strlen = cmdline->header.size - sizeof(cmdline->header);
	// 	print_str("\ncmdline->string length: "); print_num(strlen);
	// }

	// print_newline();
	// tag_header = (struct MB_tag_header*)((uint8_t*)tag_header + ((tag_header->size + 7) & ~7));
	// print_str("\ntag_header->type: "); print_num(tag_header->type);
	// print_str("\ntag_header->size: "); print_num(tag_header->size);

	// print_newline();
	// tag_header = (struct MB_tag_header*)((uint8_t*)tag_header + ((tag_header->size + 7) & ~7));
	// print_str("\ntag_header->type: "); print_num(tag_header->type);
	// print_str("\ntag_header->size: "); print_num(tag_header->size);

	// print_newline();
	// tag_header = (struct MB_tag_header*)((uint8_t*)tag_header + ((tag_header->size + 7) & ~7));
	// print_str("\ntag_header->type: "); print_num(tag_header->type);
	// print_str("\ntag_header->size: "); print_num(tag_header->size);

	// struct MB_tag_mmap* mem_map = (struct MB_tag_mmap*)tag_header;
	// print_str("\nmem_map->entry_size: "); print_num(mem_map->entry_size);
	// print_str("\nmem_map->entry_version: "); print_num(mem_map->entry_version);

	// print_newline();
	// struct MB_tag_mmap_entry* mem_entry = (struct MB_tag_mmap_entry*)((uint8_t*)mem_map + ((mem_map->entry_size + 7) & ~7));
	// print_str("\nmem_entry->base_addr: "); print_num(mem_entry->base_addr);
	// print_str("\nmem_entry->length: "); print_num(mem_entry->length);
	// print_str("\nmem_entry->type: "); print_num(mem_entry->type);

	// print_newline();
	// tag_header = (struct MB_tag_header*)((uint8_t*)tag_header + ((tag_header->size + 7) & ~7));
	// print_str("\ntag_header->type: "); print_num(tag_header->type);
	// print_str("\ntag_header->size: "); print_num(tag_header->size);
	
	// print_newline();
	// tag_header = (struct MB_tag_header*)((uint8_t*)tag_header + ((tag_header->size + 7) & ~7));
	// print_str("\ntag_header->type: "); print_num(tag_header->type);
	// print_str("\ntag_header->size: "); print_num(tag_header->size);
	
	// print_newline();
	// tag_header = (struct MB_tag_header*)((uint8_t*)tag_header + ((tag_header->size + 7) & ~7));
	// print_str("\ntag_header->type: "); print_num(tag_header->type);
	// print_str("\ntag_header->size: "); print_num(tag_header->size);
	
	// print_newline();
	// tag_header = (struct MB_tag_header*)((uint8_t*)tag_header + ((tag_header->size + 7) & ~7));
	// print_str("\ntag_header->type: "); print_num(tag_header->type);
	// print_str("\ntag_header->size: "); print_num(tag_header->size);
	
	// print_newline();
	// tag_header = (struct MB_tag_header*)((uint8_t*)tag_header + ((tag_header->size + 7) & ~7));
	// print_str("\ntag_header->type: "); print_num(tag_header->type);
	// print_str("\ntag_header->size: "); print_num(tag_header->size);
	
	// print_newline();
	// tag_header = (struct MB_tag_header*)((uint8_t*)tag_header + ((tag_header->size + 7) & ~7));
	// print_str("\ntag_header->type: "); print_num(tag_header->type);
	// print_str("\ntag_header->size: "); print_num(tag_header->size);
}

struct MB_tag_header* find_tag(struct MB_header* mb_info_start, uint32_t type) 
{
	struct MB_tag_header* tag = (struct MB_tag_header*)((uint8_t*)mb_info_start + sizeof(mb_info_start));
	while(tag->type != 0) {
		if(tag->type == type) return tag;
		tag = (struct MB_tag_header*)((uint8_t*)tag + ROUNDUP8(tag->size));
	}
	return NULL;
}

struct MB_tag_header* find_next(struct MB_tag_header* current) 
{
	if(current->type == 0) return NULL;
	struct MB_tag_header* tag = (struct MB_tag_header*)((uint8_t*)current + ROUNDUP8(current->size));
	
	if(tag->type == 0) return NULL;
	return tag;
}