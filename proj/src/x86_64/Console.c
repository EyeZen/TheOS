#include <Console.h>

#include <Keyboard.h>
#include <fbuff.h>
#include <RASCI.h>
#include <print.h>

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
    print_clear();
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
        console_write(ch);
    }
    return ch;
}

void console_write(char ch) {
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

void console_writeline(char* str, size_t size) {
    kprintf("Writing msg to console, size: %d\n", size);
    for(size_t i=0; i < size; i++) {
        // TODO: why is 0 mapping to space?
        // if(*(str + i) == 0) break;
        console_write(str[i]);
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
    print_clear();
    size_t start = console_view * console_max_chars;
    while(start < console_cursor) {
        kputchar(console_buffer[start]);
        start++;
    }
}