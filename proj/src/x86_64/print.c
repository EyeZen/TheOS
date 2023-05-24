#include "print.h"
#include <stdarg.h>

#include "Fonts.h"
#include "RASCI.h"
#include "fbuff.h"

unsigned char _KCONSOLE_TYPE;

static const size_t NUM_COLS = 80;
static const size_t NUM_ROWS = 25;

struct Char {
    uint8_t character;
    uint8_t color; // background(4-bit) foreground(4-bit) 
};

struct Char* buffer = (struct Char*) 0xb8000;
size_t col = 0;
size_t row = 0;

uint32_t console_cursor_x = 0;
uint32_t console_cursor_y = 0;
Font* console_font = &rasci_font;
static uint32_t console_bg = BLACK;
static uint32_t console_fg = WHITE; 

void console_putchar(char symbol) {
    if(symbol != '\n' && (symbol <  0x20 || (unsigned char)symbol >= console_font->num_glyphs)) return;
    if(symbol == '\n') {
        console_cursor_x = 0;
        console_cursor_y++;
    } else {
        fb_putchar(symbol, console_cursor_x, console_cursor_y, console_fg, console_bg, console_font);
        console_cursor_x++;
    }

    // if(console_cursor_x >= framebuffer.pitch / ((console_font->glyph_width * framebuffer.bpp)/8)) {
    //     console_cursor_x = 0;
    //     console_cursor_y++;
    // }
    // if(console_cursor_y > (framebuffer.buffer_size / console_font->glyph_height)) 
    //     console_cursor_y = 0;
}

uint8_t color = PRINT_COLOR_WHITE | (PRINT_COLOR_BLACK << 4);

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

int init_serial() {
    out(PORT + 1, 0x00);    // Disable all interrupts
    out(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    out(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    out(PORT + 1, 0x00);    //                  (hi byte)
    out(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    out(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold   
    out(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
    out(PORT + 4, 0x1E);    // Set in loopback mode, test the serial chip        
    out(PORT + 0, 0xAE);    // Test serial chip (send byte 0xAE and check if serial returns same byte)

    // Check if serial is faulty (i.e: not same byte as sent)
    if(in(PORT + 0) != 0xAE) {
        return 1;
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    out(PORT + 4, 0x0F);
    return 0;
}

void console_set_type(unsigned char type) {
    if(type < CONSOLE_TYPE_LIMIT) {
        _KCONSOLE_TYPE = type;
    }
}

unsigned char console_get_type() {
    return _KCONSOLE_TYPE;
}

void clear_row(size_t row) {
    // struct Char empty = (struct Char) {
    //     character: ' ',
    //     color: color,
    // };

    struct Char empty;
    empty.character = ' '; empty.color = color;

    for(size_t col = 0; col < NUM_COLS; col++) {
        buffer[col + NUM_COLS * row] = empty;
    }
}

void print_clear(uint32_t foreground, uint32_t background) {
    console_bg = background;
    if(_KCONSOLE_TYPE == CONSOLE_TYPE_SCREEN) {
        for (size_t i = 0; i < NUM_ROWS; i++) {
            clear_row(i);
        }
    } else if(_KCONSOLE_TYPE == CONSOLE_TYPE_FRAMEBUFFER) {
        fb_clear(console_bg);
        console_cursor_x = console_cursor_y = 0;
    } else {
        kprintf("print-clear: Invalid Console Type");
    }
}

void print_newline() {
    col = 0;
    if(row < NUM_ROWS - 1) {
        row++;
        return;
    }

    for(size_t row = 1; row < NUM_ROWS; row++) {
        for(size_t col = 0; col < NUM_COLS; col++) {
            struct Char character = buffer[col + NUM_COLS * row];
            buffer[col + NUM_COLS * (row - 1)] = character;
        }
    }

    clear_row(NUM_COLS - 1);
}

void kputchar(char character) {
    if(_KCONSOLE_TYPE == CONSOLE_TYPE_LOG) {
        out(PORT, character);
        return;
    } else if(_KCONSOLE_TYPE == CONSOLE_TYPE_FRAMEBUFFER) {
        console_putchar(character);
        return;
    }

    if(character == '\n') {
        print_newline();
        return;
    }

    if(col > NUM_COLS) {
        print_newline();
    }

    buffer[col + NUM_COLS * row] = (struct Char) {
        character: (uint8_t) character,
        color: color,
    };

    col++;
}

void print_set_color(uint32_t foreground, uint32_t background) {
    if(_KCONSOLE_TYPE == CONSOLE_TYPE_SCREEN)
        color = (uint8_t)foreground + ((uint8_t)background << 4);
    else if(_KCONSOLE_TYPE == CONSOLE_TYPE_FRAMEBUFFER) {
        console_fg = foreground;
        console_bg = background;
    }
}

void print_string(const char* str) {
    while (*str) {
        kputchar(*str++);
    }
}
// %d
void print_decimal(int value) {
    if (value < 0) {
        kputchar('-');
        value = -value;
    }
    if (value >= 10) {
        print_decimal(value / 10);
    }
    kputchar('0' + (value % 10));
}
// %x
void print_hexadecimal(unsigned int value) {
    const char* hex_chars = "0123456789abcdef";
    if (value >= 16) {
        print_hexadecimal(value / 16);
    }
    kputchar(hex_chars[value % 16]);
}
// %o
void print_octal(unsigned int value) {
    if (value >= 8) {
        print_octal(value / 8);
    }
    kputchar('0' + (value % 8));
}
// %u
void print_unsigned(unsigned int value) {
    if (value >= 10) {
        print_unsigned(value / 10);
    }
    kputchar('0' + (value % 10));
}
// %l
void print_long(long value) {
    if (value < 0) {
        kputchar('-');
        value = -value;
    }
    if (value >= 10) {
        print_long(value / 10);
    }
    kputchar('0' + (value % 10));
}
// %ul
void print_unsigned_long(unsigned long value) {
    if (value >= 10) {
        print_unsigned_long(value / 10);
    }
    kputchar('0' + (value % 10));
}
// %ull
void print_unsigned_long_long(unsigned long long value) {
    if (value >= 10) {
        print_unsigned_long_long(value / 10);
    }
    kputchar('0' + (value % 10));
}

void kprintf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    while (*format) {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 'c':
                    kputchar(va_arg(args, int));
                    break;

                case 's':
                    print_string(va_arg(args, const char*));
                    break;

                case 'd':
                    print_decimal(va_arg(args, int));
                    break;

                case 'x':
                    print_hexadecimal(va_arg(args, unsigned int));
                    break;

                case 'o':
                    print_octal(va_arg(args, unsigned int));
                    break;
                    
                case 'u':
                    if(*(format + 1) == 'l') {
                        if(*(format + 2) == 'l') {
                            print_unsigned_long_long(va_arg(args, unsigned long long));
                            format += 1;
                        }
                        else print_unsigned_long(va_arg(args, unsigned long));
                        format += 1;
                    } else {
                        print_unsigned(va_arg(args, unsigned int));
                    }
                    break;
                    
                case 'l':
                    print_long(va_arg(args, long));
                    break;

                default:
                    kputchar(*format);
                    break;
            }
        } else {
            kputchar(*format);
        }

        format++;
    }

    va_end(args);
}

