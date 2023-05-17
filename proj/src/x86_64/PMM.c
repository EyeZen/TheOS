#include "PMM.h"
#include "utils.h"
#include "multiboot.h"
#include "Logging.h"

struct PMM_t phys_mem;

// TODO: map bitmap frame in physical memory
void pmm_setup(struct multiboot_info_header* mbi_header)
{
    // mb-header loaded roughly at above 1MB memory, in uppor region
    // assuming everything in lower region and memory hole as reserved
    size_t lower_limit = (size_t)mbi_header + mbi_header->total_size;
    // determine memory required for bitmap
    struct multiboot_tag_basic_meminfo* meminfo = (struct multiboot_tag_basic_meminfo*)find_tag(mbi_header, MULTIBOOT_TAG_TYPE_BASIC_MEMINFO);
    phys_mem.total_memory = (meminfo->mem_upper + 1024) * 1024;   // total memory in bytes
    phys_mem.frame_count = phys_mem.total_memory / FRAME_SIZE + 1;
    phys_mem.frames_used = 0;
    phys_mem.bitmap.size = phys_mem.frame_count / BITS_PER_ROW + 1; // bitmap size in type-of(memory_map[0]) units
    size_t bitmap_size_in_bytes = phys_mem.frame_count / 8 + 1; // bitmap size in bytes


    // take mmap struture, and find available memory space for bitmap
    struct multiboot_tag_mmap* mmap = (struct multiboot_tag_mmap*)find_tag(mbi_header, MULTIBOOT_TAG_TYPE_MMAP);
    phys_mem.entries = mmap->entries;
    phys_mem.entries_count = (mmap->size - sizeof(struct multiboot_tag_mmap)) / mmap->entry_size;

    phys_mem.bitmap.memory_map = NULL;
    for(size_t i=0; i < phys_mem.entries_count; i++) 
    {
        struct multiboot_mmap_entry* current_entry = (phys_mem.entries + i);
        if(current_entry->type != MULTIBOOT_MEMORY_AVAILABLE)
            continue;
        // only available region below lower_limit(specific) is lower memory region
        // safe to assume if a part lies in upper half, then region lies in upper half
        if(current_entry->addr + current_entry->len < lower_limit)
            continue;
        
        size_t available_memory = current_entry->len;
        size_t offset = 0;
        if(lower_limit > current_entry->addr) offset = lower_limit;

        available_memory -= offset;
        if(available_memory >= bitmap_size_in_bytes) {
            phys_mem.bitmap.memory_map = (uint64_t*)(current_entry->addr + offset);
        }
    }

    if(phys_mem.bitmap.memory_map == NULL) {
        logf("\nNo memory available for pmm bitmap!");
        return;
    }

    // allocate bitmap, mark all memory available
    for(size_t i = 0; i < phys_mem.frame_count; i++) {
        phys_mem.bitmap.memory_map[i] = 0;
    }
    // mark kernel area used
    size_t kernel_frames_count = lower_limit / FRAME_SIZE;
    if(lower_limit != 0) {
        kernel_frames_count++;
    }
    // TODO: ? allocate 1 additional frame for bitmap and page tables?! not too sure
    kernel_frames_count++;
    size_t j = 0;
    for(size_t j=0; j < ROW_INDEX(kernel_frames_count); j++) {
        phys_mem.bitmap.memory_map[j] = ~(0);
    }
    // set initial frames in next, partial empty row-entry
    phys_mem.bitmap.memory_map[j] = ~(~(0ul) << COL_INDEX(kernel_frames_count));
    phys_mem.frames_used = kernel_frames_count;

    // mark bitmap area used
    pmm_reserve_area((uint64_t)phys_mem.bitmap.memory_map, bitmap_size_in_bytes);

    // size_t max_frame = 0;
    // for(size_t i=0; i < phys_mem.frame_count; i++) {
    //     if(_bitmap_test_bit(i)) max_frame = i;
    // }

    // log_str("\nMaximum Frame Address: ");
    // log_num(FRAME_ADDRESS(max_frame));
}

/**
 * This function allocate a physical frame of memory.
 * 
 * @return The physical address of the allocated frame of memory of PAGE_SIZE_IN_BYTES
 */
void *pmm_alloc_frame(){
    if( ! pmm_check_frame_availability() ) {
        return 0; // No more frames to allocate
    }

    uint64_t frame = _bitmap_request_frame();
    if (frame > 0) {
        _bitmap_set_bit(frame);
        phys_mem.frames_used++;
        return (void*)FRAME_ADDRESS(frame);
    }
    return NULL;
}

void pmm_free_frame(void *address){
    uint64_t frame = FRAME_POS1((uint64_t)address);
    _bitmap_free_bit(frame);
    phys_mem.frames_used--;
}

bool pmm_check_frame_availability() {
    if(phys_mem.frames_used < phys_mem.frame_count){
        return true;
    }
    return false; 
}

void pmm_reserve_area(uint64_t starting_address, size_t size){
    uint64_t location = FRAME_POS1(starting_address);
    uint32_t number_of_frames = size / FRAME_SIZE;
    if((size % FRAME_SIZE) != 0){
        number_of_frames++;
    }
    for(; number_of_frames > 0; number_of_frames--){
       if(!_bitmap_test_bit(location)){
           _bitmap_set_bit(location++);
           phys_mem.frames_used++;
       }
    }
}

void pmm_free_area(uint64_t starting_address, size_t size){
    uint64_t location = FRAME_POS1(starting_address);

    if(test_reserved(starting_address)) 
        return;

    uint32_t number_of_frames = size / FRAME_SIZE;
    if((size % FRAME_SIZE) != 0){
        number_of_frames++;
    }
    for(; number_of_frames > 0; number_of_frames--){
        _bitmap_free_bit(location);
        phys_mem.frames_used--;
    }
}

bool test_reserved(uint64_t address) {
    for(size_t i = 0; i < phys_mem.entries_count; i++) {
        struct multiboot_mmap_entry* current_entry = (phys_mem.entries + i);
        // check for inclusion in reserved memory regions
        if(current_entry->type > MULTIBOOT_MEMORY_AVAILABLE) {
            uint64_t base_addr = current_entry->addr;
            uint64_t len = current_entry->len;

            if(address >= base_addr && address < (base_addr + len)) 
                return true;
        }
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////

/**
 * This function is returning the bitmap location of the first available page-frame
 *
 * */
int64_t _bitmap_request_frame(){
    for (size_t row = 0, column = 0; row < phys_mem.bitmap.size; row++){
        if(phys_mem.bitmap.memory_map[row] != BITMAP_ENTRY_FULL){
            for (column = 0; column < BITS_PER_ROW; column++){
                uint64_t bit = 1 << column;
                if((phys_mem.bitmap.memory_map[row] & bit) == 0){
                    //Found a location
                    return FRAME_POS2(row, column);
                }
            }
        }
    }
    return -1;
}

void _bitmap_set_bit_from_address(uint64_t address) {
    if( address < (phys_mem.frame_count * FRAME_SIZE) ) {
        _bitmap_set_bit(FRAME_POS1(address));
    }
}

/**
 * In the next 3 function location is the bit-location inside the bitmap
 * */
void _bitmap_set_bit(uint64_t location){
    phys_mem.bitmap.memory_map[ROW_INDEX(location)] |= 1 << COL_INDEX(location);
}

void _bitmap_free_bit(uint64_t location){
    phys_mem.bitmap.memory_map[ROW_INDEX(location)] &= ~(1 << COL_INDEX(location));
}

bool _bitmap_test_bit(uint64_t location){
    return phys_mem.bitmap.memory_map[ROW_INDEX(location)] & (1 << COL_INDEX(location));
}
