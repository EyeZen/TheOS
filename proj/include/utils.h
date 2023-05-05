#pragma once

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