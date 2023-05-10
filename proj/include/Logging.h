#ifndef LOGGING_H
#define LOGGING_H

#include "multiboot.h"
#include <stdint.h>

#define PORT 0x3f8

int init_serial();

void log_char(char ch);
void log_str(char *string);
void log_num(unsigned int num);

void log_achar(char ch);
void log_astr(char *string);
void log_anum(unsigned int num);

void log_tag(struct multiboot_tag* tag);
void log_mbheader(struct multiboot_info_header* mboot_header);
void log_page_table(uint64_t* pml4t);

void block_start();
void block_end();
void reset_block();

#define LOG_CALL(func) do { \
    log_astr(#func); log_endl(); \
    func(); \
} while (0)

#endif