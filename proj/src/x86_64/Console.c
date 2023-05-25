#include <Console.h>
#include <stdarg.h>

#include <Keyboard.h>
#include <fbuff.h>
#include <RASCI.h>
#include <print.h>
#include <utils.h>
#include <Logging.h>

#define CONSOLE_BUF_MAX_SIZE 16000

char console_buffer[CONSOLE_BUF_MAX_SIZE];
size_t console_view;
size_t console_cursor;
key_status current_key;
size_t console_max_chars;

void clear_console() {
    console_view = 0;
    console_cursor = 0;
    current_key.code = UNDEFINED;
    print_clear(WHITE, BLACK);
}

void console_init() {
    console_max_chars = framebuffer.buffer_size / rasci_font.glyph_size;
    logf("char size(bytes): %d\n", rasci_font.glyph_size);
    logf("console max chars: %d\n", console_max_chars);
    clear_console();
}

char console_read(bool echo) {
    char ch;
    do {
        current_key = get_next_key();
        if(current_key.code != UNDEFINED)
            ch = kgetch(current_key);
    } while(current_key.code == UNDEFINED || ch == 0);

    if(echo == true) {
        console_writechar(ch);
    }
    return ch;
}

void console_writechar(char ch) {
    size_t console_pos = console_cursor % CONSOLE_BUF_MAX_SIZE;
    console_cursor++;
    console_buffer[console_pos] = ch;
    // update view
    update_view();
    // update framebuffer
    kputchar(ch);
}

size_t console_readline(char* str, size_t max_size, bool echo) {
    char ch;
    size_t i=0;  

    do {
        ch = console_read(echo);
        if(i < max_size) {
            str[i] = ch;
            i++;
        }
    } while(ch != '\n');
    str[i] = 0;
    // for(i=0; i < max_size-1; i++) {
    //     ch = console_read(echo);
    //     if(ch == '\n') break;
    //     *(str+i) = ch;
    // }
    // *(str + i) = 0;
    return i;
}


// void print_string(const char* str) {
//     while (*str) {
//         console_write(*str++);
//     }
// }
// %d
void write_decimal(int value) {
    if (value < 0) {
        console_writechar('-');
        value = -value;
    }
    if (value >= 10) {
        write_decimal(value / 10);
    }
    console_writechar('0' + (value % 10));
}
// %x
void write_hexadecimal(unsigned int value) {
    const char* hex_chars = "0123456789abcdef";
    if (value >= 16) {
        write_hexadecimal(value / 16);
    }
    console_writechar(hex_chars[value % 16]);
}
// %o
void write_octal(unsigned int value) {
    if (value >= 8) {
        write_octal(value / 8);
    }
    console_writechar('0' + (value % 8));
}
// %u
void write_unsigned(unsigned int value) {
    if (value >= 10) {
        write_unsigned(value / 10);
    }
    console_writechar('0' + (value % 10));
}
// %l
void write_long(long value) {
    if (value < 0) {
        console_writechar('-');
        value = -value;
    }
    if (value >= 10) {
        write_long(value / 10);
    }
    console_writechar('0' + (value % 10));
}
// %ul
void write_unsigned_long(unsigned long value) {
    if (value >= 10) {
        write_unsigned_long(value / 10);
    }
    console_writechar('0' + (value % 10));
}
// %ull
void write_unsigned_long_long(unsigned long long value) {
    if (value >= 10) {
        write_unsigned_long_long(value / 10);
    }
    console_writechar('0' + (value % 10));
}

void console_write(const char* format, ...) {
    va_list args;
    va_start(args, format);

    while (*format) {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 'c':
                    console_writechar(va_arg(args, int));
                    break;

                case 's':
                    console_writeline(va_arg(args, const char*));
                    break;

                case 'd':
                    write_decimal(va_arg(args, int));
                    break;

                case 'x':
                    write_hexadecimal(va_arg(args, unsigned int));
                    break;

                case 'o':
                    write_octal(va_arg(args, unsigned int));
                    break;
                    
                case 'u':
                    if(*(format + 1) == 'l') {
                        if(*(format + 2) == 'l') {
                            write_unsigned_long_long(va_arg(args, unsigned long long));
                            format += 1;
                        }
                        else write_unsigned_long(va_arg(args, unsigned long));
                        format += 1;
                    } else {
                        write_unsigned(va_arg(args, unsigned int));
                    }
                    break;
                    
                case 'l':
                    write_long(va_arg(args, long));
                    break;

                default:
                    console_writechar(*format);
                    break;
            }
        } else {
            console_writechar(*format);
        }

        format++;
    }

    va_end(args);
}


void console_writeline(const char* str) {
    size_t size = strlen(str);    
    for(size_t i=0; i < size; i++) {
        // TODO: why is 0 mapping to space?
        // if(*(str + i) == 0) break;
        console_writechar(str[i]);
    }
}

void update_view() {
    size_t fb_idx = (console_cursor+1) / console_max_chars;
    if(((console_cursor+1) % console_max_chars) != 0) fb_idx++;

    if(fb_idx > console_view) {
        console_view = fb_idx;
        update_screen();
    }
}

void update_screen() {
    // write_clear(WHITE, BLACK);
    size_t start = console_view * console_max_chars;
    while(start < console_cursor) {
        kputchar(console_buffer[start]);
        start++;
    }
}