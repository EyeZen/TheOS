#include "utils.h"
#include "VMM.h"
#include <stddef.h>

struct multiboot_tag* find_tag(struct multiboot_info_header* mb_info_start, uint32_t type) 
{
	struct multiboot_tag* tag = (struct multiboot_tag*)((uint8_t*)mb_info_start + sizeof(mb_info_start));
	while(tag->type != MULTIBOOT_TAG_TYPE_END) {
		if(tag->type == type) return tag;
		tag = (struct multiboot_tag*)((uint8_t*)tag + ROUNDUP8(tag->size));
	}
	return NULL;
}

struct multiboot_tag* find_next(struct multiboot_tag* current) 
{
	if(current->type == MULTIBOOT_TAG_TYPE_END) return NULL;
	struct multiboot_tag* tag = (struct multiboot_tag*)((uint8_t*)current + ROUNDUP8(current->size));
	
	if(tag->type == MULTIBOOT_TAG_TYPE_END) return NULL;
	return tag;
}

// no support for RSDPDescriptorV2 or XSDT
struct SDTHeader* find_sdt(struct multiboot_info_header* mboot_header, const char* signature) {
	struct multiboot_tag_old_acpi* acpi = (struct multiboot_tag_old_acpi*)find_tag(mboot_header, MULTIBOOT_TAG_TYPE_ACPI_OLD);
	struct RSDPDescriptor* rsdp_desc = (struct RSDPDescriptor*)acpi->rsdp;
	struct RSDT* rsdt = (struct RSDT*)VPTR(rsdp_desc->RSDTAddress);

	for(uint32_t i=0; i < RSDT_ITEMS_COUNT(rsdt); i++) {
		struct SDTHeader* sdt = (struct SDTHeader*)VPTR(rsdt->sdtAddresses[i]);
		int sig_itr = 0;
		for(sig_itr=0; sig_itr < 4; sig_itr++) {
			if(sdt->Signature[sig_itr] != signature[sig_itr]) break;
		}
		if(sig_itr == 4)
			return sdt;
	}

	return NULL;
}

struct MADTEntry* find_madt_record(struct MADT* madt, uint8_t type, uint32_t offset) {
	struct MADTEntry* entry = (struct MADTEntry*)((uint64_t)madt + sizeof(struct MADT));
	
	uint64_t scanned_length=sizeof(struct MADT);
	uint32_t count = 0;
	while(scanned_length < madt->sdtHeader.Length) {
		if(entry->type == type) {
			if(count == offset) {
				return entry;
			}
			count++;
		}

		scanned_length += entry->length;
		entry = (struct MADTEntry*)((uint64_t)entry + entry->length);
	}	

	return NULL;
}

void strcpy(char* src, char* dst) {
    while(*src) {
        *dst++ = *src++;
    }
}

size_t strncpy(char* dst, char* src, size_t size)
{
	size_t i;
	for(i=0; i < size && *dst && *src; i++) {
		*(dst + i) = *(src + i);
	}
	return i;
}

int strlen(char* str) {
	int len = 0;
	while(str[len++] != 0);
	return len;
}

int strcmp(char* str1, char* str2, size_t size) {
	if(size == 0) {
		int len1 = strlen(str1);
		int len2 = strlen(str2);
		if(len1 != len2) return len1 - len2;
	}
	for(size_t i=0; i < size; i++) {
		if(*str1 == 0 || *str2 == 0) return -1;
		int diff = *str1 - *str2;
		if(diff != 0) return diff;
	}

	return 0;
}

void memcpy(void* dst, void* src, size_t size) {
    for(size_t i=0; i < size; i++) {
        *(uint8_t*)dst = *((uint8_t*)src + i);
    }
}

int memset(void* ptr, size_t value, size_t size) {
	if(ptr == NULL) {
		return -1;
	}
	for(size_t i=0; i < size; i++) { 
		*((char*)ptr + i) = value; 
	}
	return 0;
}