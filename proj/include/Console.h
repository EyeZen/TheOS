#ifndef _CONSOLE_H
#define _CONSOLE_H

#include <stddef.h>
#include <stdbool.h>

void console_init();
void clear_console();
char console_read(bool echo);
void console_write(char ch);
size_t console_readline(char* str, size_t max_size, bool echo);
void console_writeline(char* str);

void update_view();
void update_screen();

#endif