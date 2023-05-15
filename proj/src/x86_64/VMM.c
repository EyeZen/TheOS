#include "VMM.h"
#include "MEM.h"
#include "PMM.h"
#include <stdint.h>

void *map_phys_to_virt_addr(void* physical_address, void* address, unsigned int flags)
{
    uint64_t pml4_idx = PML4_INDEX((uint64_t)address);
    uint64_t pdp_idx  = PDP_INDEX((uint64_t)address);
    uint64_t pd_idx   = PD_INDEX((uint64_t)address);
    uint64_t pt_idx   = PT_INDEX((uint64_t)address);

    uint64_t* pml4_table = (uint64_t*)(SIGN_EXTENSION | ENTRIES_TO_ADDRESS(511L, 511L, 511L, 511L));
    uint64_t* pdp_table = (uint64_t*)(SIGN_EXTENSION | ENTRIES_TO_ADDRESS(511L, 511L, 511L, pml4_idx));
    uint64_t* pd_table = (uint64_t*)(SIGN_EXTENSION | ENTRIES_TO_ADDRESS(511L, 511L, pml4_idx, pdp_idx));
    uint64_t* page_table = (uint64_t*)(SIGN_EXTENSION | ENTRIES_TO_ADDRESS(511L, pml4_idx, pdp_idx, pd_idx));

    if(!(pml4_table[pml4_idx] & PRESENT_BIT)) {
        // allocate frame for new pdp table
        uint64_t* new_pdp_frame_phys_addr = (uint64_t*)pmm_alloc_frame();
        // add new table entry to pml4 table
        pml4_table[pml4_idx] = (uint64_t)new_pdp_frame_phys_addr | WRITE_BIT | PRESENT_BIT;
        // new entry sits where pdp_table should be, effectively becoming pdp_table value
        clean_new_table(pdp_table);
    }

    if(!(pdp_table[pdp_idx] & PRESENT_BIT)) {
        uint64_t* new_pd_frame_phy_addr = (uint64_t*)pmm_alloc_frame();
        pdp_table[pdp_idx] = (uint64_t)new_pd_frame_phy_addr | WRITE_BIT | PRESENT_BIT;
        clean_new_table(pd_table);
    }

    if(!(pd_table[pd_idx] & PRESENT_BIT)) {
        uint64_t* new_pt_frame_phy_addr = (uint64_t*)pmm_alloc_frame();
        pd_table[pd_idx] = (uint64_t)new_pt_frame_phy_addr | WRITE_BIT | PRESENT_BIT;
        clean_new_table(page_table);
    }

    if(!(page_table[pt_idx] & PRESENT_BIT)) {
        page_table[pt_idx] = (uint64_t)physical_address | flags | WRITE_BIT | PRESENT_BIT;
    }

    return address;
}

void *map_vaddress(void *virtual_address, unsigned int flags)
{
    void* new_phy_addr = pmm_alloc_frame();
    return map_phys_to_virt_addr(new_phy_addr, virtual_address, flags);
}

void map_vaddress_range(void *virtual_address, unsigned int flags, size_t required_pages)
{
    for(size_t i = 0; i < required_pages; i++) {
        map_vaddress((void*)(virtual_address + (i * PAGE_SIZE)), flags);
    }
}

void identity_map_phys_address(void *physical_address, unsigned int flags) {
    map_phys_to_virt_addr(physical_address, physical_address, flags);
}

int unmap_vaddress(void *address) 
{
    uint64_t* pml4_table = (uint64_t*)(SIGN_EXTENSION | ENTRIES_TO_ADDRESS(511L, 511L, 511L, 511L));
    uint64_t pml4_idx = PML4_INDEX((uint64_t)address);
    if(!(pml4_table[pml4_idx] & PRESENT_BIT)) {
        return -1;
    }

    uint64_t* pdp_table = (uint64_t*)(SIGN_EXTENSION | ENTRIES_TO_ADDRESS(511L, 511L, 511L, pml4_idx));
    uint64_t pdp_idx = PDP_INDEX((uint64_t)address);
    if(!(pdp_table[pdp_idx] & PRESENT_BIT)) {
        return -1;
    }

    uint64_t* pd_table = (uint64_t*)(SIGN_EXTENSION | ENTRIES_TO_ADDRESS(511L, 511L, pml4_idx, pdp_idx));
    uint64_t pd_idx = PD_INDEX((uint64_t)address);
    if(!(pd_table[pd_idx] & PRESENT_BIT)) {
        return -1;
    }

    uint64_t* page_table = (uint64_t*)(SIGN_EXTENSION | ENTRIES_TO_ADDRESS(511L, pml4_idx, pdp_idx, pd_idx));
    uint64_t pt_idx = PT_INDEX((uint64_t)address);
    if(!(page_table[pt_idx] & PRESENT_BIT)) {
        return -1;
    }

    page_table[pt_idx] = 0x0L;
    invalidate_page_table((uint64_t*)address);

    return 0;
}

uint8_t is_phyisical_address_mapped(uint64_t physical_address, uint64_t virtual_address)
{
    uint64_t* pml4_table = (uint64_t*)(SIGN_EXTENSION | ENTRIES_TO_ADDRESS(511L, 511L, 511L, 511L));
    uint64_t pml4_idx = PML4_INDEX((uint64_t)virtual_address);
    if(!(pml4_table[pml4_idx] & PRESENT_BIT)) {
        return ADDRESS_NOT_MAPPED;
    }

    uint64_t* pdp_table = (uint64_t*)(SIGN_EXTENSION | ENTRIES_TO_ADDRESS(511L, 511L, 511L, pml4_idx));
    uint64_t pdp_idx = PDP_INDEX((uint64_t)virtual_address);
    if(!(pdp_table[pdp_idx] & PRESENT_BIT)) {
        return ADDRESS_NOT_MAPPED;
    }

    uint64_t* pd_table = (uint64_t*)(SIGN_EXTENSION | ENTRIES_TO_ADDRESS(511L, 511L, pml4_idx, pdp_idx));
    uint64_t pd_idx = PD_INDEX((uint64_t)virtual_address);
    if(!(pd_table[pd_idx] & PRESENT_BIT)) {
        return ADDRESS_NOT_MAPPED;
    }

    uint64_t* page_table = (uint64_t*)(SIGN_EXTENSION | ENTRIES_TO_ADDRESS(511L, pml4_idx, pdp_idx, pd_idx));
    uint64_t pt_idx = PT_INDEX((uint64_t)virtual_address);
    if(!(page_table[pt_idx] & PRESENT_BIT)) {
        return ADDRESS_NOT_MAPPED;
    }

    if(ALIGN_PHYSADDRESS(page_table[pt_idx]) != ALIGN_PHYSADDRESS(physical_address))
        return ADDRESS_MISMATCH;

    return ADDRESS_MAPPED;
}