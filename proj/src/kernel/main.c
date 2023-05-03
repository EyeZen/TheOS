#include "print.h"

// called inside main64.asm
void kernel_main() {
    print_clear();
    print_set_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);
    print_str("Welcome to our 64-bit kernel");
}