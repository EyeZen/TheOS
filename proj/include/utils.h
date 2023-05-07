#pragma once


#define ROUNDUP8(addr) ((addr + 7) & ~7)
#define FIRST_TAG(mboot) ((struct multiboot_tag*)((uint8_t*)mboot + sizeof(struct multiboot_info_header)))

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


void dec_to_hex(unsigned int num, char* hex_str)
{
    char hex_digits[] = "0123456789ABCDEF";
    int i = 0;
    
    do {
        hex_str[i++] = hex_digits[num % 16];
        num /= 16;
    } while (num > 0);
    
    // Reverse the string
    int j, k;
    char temp;
    for (j = 0, k = i-1; j < k; j++, k--) {
        temp = hex_str[j];
        hex_str[j] = hex_str[k];
        hex_str[k] = temp;
    }
    
    // Add the "0x" prefix
    hex_str[i++] = 'x';
    hex_str[i++] = '0';
    hex_str[i] = '\0';
}

void strcpy(char* src, char* dst) {
    while(*src) {
        *dst++ = *src++;
    }
}