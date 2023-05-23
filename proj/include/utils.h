#ifndef _UTILS_H
#define _UTILS_H

#include "multiboot.h"
#include <stdint.h>
#include "ACPI.h"

#define ROUNDUP8(addr) ((addr + 7) & ~7)
#define FIRST_TAG(mboot) ((struct multiboot_tag*)((uint8_t*)mboot + sizeof(struct multiboot_info_header)))

struct multiboot_tag* find_tag(struct multiboot_info_header* mb_info_start, uint32_t type);

struct multiboot_tag* find_next(struct multiboot_tag* current);

struct SDTHeader* find_sdt(struct multiboot_info_header* mboot_header, const char* signature);

struct MADTEntry* find_madt_record(struct MADT* madt, uint8_t type, uint32_t offset);

void strcpy(char* src, char* dst);

size_t strncpy(char* dst, char* src, size_t size);

int strlen(char* str);

int strcmp(char* str1, char* str2, size_t size);

void memcpy(void* dst, void* src, size_t size);

int memset(void* ptr, size_t value, size_t size);

#endif