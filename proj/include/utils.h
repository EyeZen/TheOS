#ifndef _UTILS_H
#define _UTILS_H

#include "multiboot.h"
#include <stdint.h>

#define ROUNDUP8(addr) ((addr + 7) & ~7)
#define FIRST_TAG(mboot) ((struct multiboot_tag*)((uint8_t*)mboot + sizeof(struct multiboot_info_header)))

struct multiboot_tag* find_tag(struct multiboot_info_header* mb_info_start, uint32_t type);

struct multiboot_tag* find_next(struct multiboot_tag* current);


int dec_to_hex(unsigned int num, char* hex_str);

void strcpy(char* src, char* dst);

#endif