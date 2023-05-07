#ifndef LOGGING_H
#define LOGGING_H

#include "multiboot.h"

#define PORT 0x3f8


inline unsigned char in(int portnum)
{
    unsigned char data=0;
    __asm__ __volatile__ ("inb %%dx, %%al" : "=a" (data) : "d" (portnum));       
    return data;
}

inline void out(int portnum, unsigned char data)
{
    __asm__ __volatile__ ("outb %%al, %%dx" :: "a" (data),"d" (portnum));        
}

int init_serial();

void log_char(char ch);
void log_str(char *string);
void log_num(unsigned int num);

void log_achar(char ch);
void log_astr(char *string);
void log_anum(unsigned int num);

void log_tag(struct multiboot_tag* tag);
void log_mbheader(struct multiboot_info_header* mboot_header);

inline void block_start();
inline void block_end();
inline void reset_block();

#endif