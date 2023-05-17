#ifndef _MEM_H
#define _MEM_H

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 0x1000              // 4KiB

#define TABLE_OFFSET_MASK   0x1ffL    // 9 bits table offsets
#define DATA_OFFSET_MASK    0xfffL    // 12 bits data offsets

#define PML4_INDEX(vaddress)    ((vaddress >> 39) & TABLE_OFFSET_MASK)
#define PDP_INDEX(vaddress)     ((vaddress >> 30) & TABLE_OFFSET_MASK)
#define PD_INDEX(vaddress)      ((vaddress >> 21) & TABLE_OFFSET_MASK)
#define PT_INDEX(vaddress)      ((vaddress >> 12) & TABLE_OFFSET_MASK)
#define DATA_OFFSET(vaddress)   (vaddress & DATA_OFFSET_MASK)
#define SIGN_EXTENSION 0xFFFF000000000000
#define ENTRIES_TO_ADDRESS(pml4, pdpr, pd, pt)((pml4 << 39) | (pdpr << 30) | (pd << 21) |  (pt << 12))


#define PAGE_ALIGNMENT_MASK (PAGE_SIZE-1)   // 0x1000 - 1 = 0xFFF, mask used to clear last 3 nibbles to align to page boundary

#define ALIGN_PHYSADDRESS(address)(address & (~(PAGE_ALIGNMENT_MASK)))

#define PRESENT_BIT 1
#define WRITE_BIT 0b10

#define VM_PAGES_PER_TABLE 0x200    // 512

#define VM_OFFSET_MASK 0xFFFFFFFFFFFFF000
#define PAGE_ENTRY_FLAGS PRESENT_BIT | WRITE_BIT

#define KiB 0x400L
#define MiB (KiB * KiB)
#define GiB (KiB * KiB * KiB)


#define READMEM32(addr) \
(*(volatile uint32_t*)((uintptr_t)addr))

#define READMEM64(addr) \
(*(volatile uint64_t*)((uintptr_t)addr))

#define WRITEMEM32(addr, u32) \
(*(volatile uint32_t*)((uintptr_t)addr)) = u32

#define WRITEMEM64(addr, u64) \
(*(volatile uint64_t*)((uintptr_t)addr)) = u64


void clean_new_table(uint64_t *);

void invalidate_page_table(uint64_t *);

void load_page_table(void*);

#endif