#ifndef _VMM_H
#define _VMM_H

#include <stdint.h>
#include <stddef.h>

#define ADDRESS_NOT_MAPPED  0
#define ADDRESS_MAPPED      1
#define ADDRESS_MISMATCH    2

#define VPTR(x) (void*)((uint64_t)(x))

void *map_phys_to_virt_addr(void* physical_address, void* address, unsigned int flags);

void *map_vaddress(void *virtual_address, unsigned int flags);

void map_vaddress_range(void *virtual_address, unsigned int flags, size_t required_pages);

void identity_map_phys_address(void *physical_address, unsigned int flags);

int unmap_vaddress(void *address);

uint8_t is_phyisical_address_mapped(uint64_t physical_address, uint64_t virtual_address);

#endif