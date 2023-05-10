#include "MEM.h"

void clean_new_table( uint64_t *table_to_clean ) {
    for(int i = 0; i < VM_PAGES_PER_TABLE; i++){
        table_to_clean[i] = 0x00l;
    }
}

void load_page_table( void* cr3_value ) {
    //invalidate_page_table((uint64_t) cr3_value);
    asm volatile("mov %0, %%cr3" :: "r"((uint64_t)cr3_value) : "memory");
}


void invalidate_page_table( uint64_t *table_address ) {
	asm volatile("invlpg (%0)"
    	:
    	: "r"((uint64_t)table_address)
    	: "memory");
}