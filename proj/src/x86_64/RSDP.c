#include "RSDP.h"

uint8_t validate_RSDP(char* byte_array, size_t size)
{
    uint32_t sum = 0;
    for(size_t i = 0; i < size; i++) {
        sum += byte_array[i];
    }
    return ((sum & 0xFF) == 0);
}