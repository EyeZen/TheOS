#ifndef _PMM_H
#define _PMM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "multiboot.h"

#define FRAME_SIZE 0x1000   // 4KiB, same as PAGE_SIZE, in bytes
#define BITS_PER_ROW 64
#define BITMAP_ENTRY_FULL 0xFFFFffffFFFFffff

#define ROW_INDEX(frame_number) (frame_number/BITS_PER_ROW)
#define COL_INDEX(frame_number) (frame_number%BITS_PER_ROW)
#define FRAME_POS1(address) (address / FRAME_SIZE)          // address to frame location / frame number
#define FRAME_POS2(row, col) (row * BITS_PER_ROW + col)     // row,column index to frame location / frame number
#define FRAME_ADDRESS(frame_number) (frame_number * FRAME_SIZE) // convert frame number/index to frame's physical address

struct Bitmap_t {
    uint64_t* memory_map;   // bitmap
    size_t size;            // bitmap number of rows/entries

}; 
struct PMM_t {
    struct Bitmap_t bitmap;
    size_t frame_count;
    size_t frames_used;
    struct multiboot_mmap_entry* entries;
    size_t entries_count;

};
extern struct PMM_t phys_mem;

void pmm_setup(struct multiboot_info_header* mbi_header);
void _map_pmm();
void *pmm_alloc_frame();
void pmm_free_frame(void*);
bool pmm_check_frame_availability();

void pmm_reserve_area(uint64_t, size_t);
void pmm_free_area(uint64_t, size_t);

int64_t _bitmap_request_frame();
void _bitmap_set_bit(uint64_t);
void _bitmap_free_bit(uint64_t);
bool _bitmap_test_bit(uint64_t);
void _bitmap_set_bit_from_address(uint64_t);

bool test_reserved(uint64_t address);

#endif