#ifndef LOGGING_H
#define LOGGING_H

#include "print.h"

#include "multiboot.h"
#include <stdint.h>
#include "IDT.h"
#include "KHeap.h"
#include "Fonts.h"

// int init_serial();

// void log_char(char ch);
// void logf(const char *string);
// void log_num(uint64_t num);

// void log_achar(char ch);
// void logfa(const char *string);
// void log_anum(uint64_t num);

void log_tag(struct multiboot_tag* tag);
void log_mbheader(struct multiboot_info_header* mboot_header);
void log_page_table(uint64_t* pml4t);
void log_context(struct cpu_status_t* context);
void log_interrupt(struct cpu_status_t* context);
void log_heap();
void log_glyph(char symbol, Font* font);
void log_font_header(uint64_t font_start_address);
void log_scheduling_queues();

void block_start();
void block_end();
void block_align();
void reset_block();

#define logf(...) \
    { \
        unsigned char type = console_get_type(); \
        console_set_type(CONSOLE_TYPE_LOG); \
        kprintf(__VA_ARGS__); \
        console_set_type(type); \
    }

/// @brief
/// @param format formatted string, same as for printf
/// @param ... variable length arguments
#define logfa(...) \
    { \
        block_align(); \
        logf(__VA_ARGS__); \
    }

#define LOG_CALL(func) do { \
    logfa(#func); log_endl(); \
    func(); \
} while (0)

#endif