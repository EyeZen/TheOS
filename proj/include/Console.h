#ifndef _CONSOLE_H
#define _CONSOLE_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

void console_init();
void clear_console();
char console_read(bool echo);
void console_writechar(char ch);
size_t console_readline(char* str, size_t max_size, bool echo);
void console_writeline(const char* str);
void console_write(const char* format, ...);

void update_view();
void update_screen();

#endif