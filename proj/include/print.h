#pragma once

#include <stdint.h>
#include <stddef.h>

#define CONSOLE_TYPE_SCREEN 0u
#define CONSOLE_TYPE_LOG 1u
#define CONSLE_TYPE_LIMIT 2u

#define PORT 0x3f8

enum {
    PRINT_COLOR_BLACK       = 0,
    PRINT_COLOR_BLUE        = 1,
    PRINT_COLOR_GREEN       = 2,
    PRINT_COLOR_CYAN        = 3,
    PRINT_COLOR_RED         = 4,
    PRINT_COLOR_MAGENTA     = 5,
    PRINT_COLOR_BROWN       = 6,
    PRINT_COLOR_LIGHT_GRAY  = 7,
    PRINT_COLOR_DARK_GRAY   = 8,
    PRINT_COLOR_LIGHT_BLUE  = 9,
    PRINT_COLOR_LIGHT_GREEN = 10,
    PRINT_COLOR_LIGHT_CYAN  = 11,
    PRINT_COLOR_LIGHT_RED   = 12,
    PRINT_COLOR_PINK        = 13,
    PRINT_COLOR_YELLOW      = 14,
    PRINT_COLOR_WHITE       = 15,
};

void print_clear();
void kputchar(char character);
// void print_str(char* string);
void print_set_color(uint8_t foreground, uint8_t background);
// void print_num(uint32_t n);
void print_newline();

////////////////////
void print_string(const char* str);
void print_decimal(int value);
void print_hexadecimal(unsigned int value);
void print_octal(unsigned int value);
void kprintf(const char* format, ...);
void print_unsigned(unsigned int value);
void print_long(long value);
void print_unsigned_long(unsigned long value);
void console_set_type(unsigned char type);
unsigned char console_get_type();

int init_serial();

