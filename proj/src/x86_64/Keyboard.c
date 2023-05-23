#include <Keyboard.h>
#include <Logging.h>
#include <print.h>
#include <ACPI.h>

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


key_status keyboard_buffer[MAX_KEYB_BUFFER_SIZE];
key_codes scancode_mappings[] = {
    0, ESCAPE, D1, D2, D3, D4, D5, D6, D7, D8, D9, D0, MINUS, EQUALS, BACKSPACE,
    TAB, Q, W, E, R, T, Y, U, I, O, P, SQBRACKET_OPEN, SQBRACKET_CLOSE, ENTER,
    LEFT_CTRL, A, S, D, F, G, H, J, K, L, SEMICOLON, SINGLE_QUOTE, BACK_TICK,
    LEFT_SHIFT, SLASH, Z, X, C, V, B, N, M, COMMA, DOT, BACKSLASH, RIGHT_SHIFT,
    KEYPAD_STAR, LEFT_ALT, SPACE, CAPS_LOCK,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, 
    F11, F12, NUM_LOCK, SCROLL_LOCK,
    KEYPAD_D7, KEYPAD_D8, KEYPAD_D9, KEYPAD_MINUS, KEYPAD_D4, KEYPAD_D5, KEYPAD_D6, KEYPAD_PLUS, KEYPAD_D1, KEYPAD_D2, KEYPAD_D3, KEYPAD_D0, KEYPAD_DOT,
};

char keymap[] = {
    0, ESCAPE, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j' , 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    0, 0, ' ', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0,
    '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.',

};

size_t buf_write_head;
size_t buf_read_head;
uint8_t current_modifiers;
bool extended_read;

uint8_t scancode_set = 0;
bool translation_enabled;

void init_keyboard() {
    // enable irq
    IOAPIC_RedirectEntry entry;
    uint8_t redtbl_idx = 0x10 + 2*PS2_TARGET_DEVICE;
    if(read_ioapic_redirect(redtbl_idx, &entry) != 0) {
        logf("keyboard-init: read-ioapic-redirect failed!\n");
    }
    entry.vector = INTR_KEYBOARD_INTERRUPT;
    entry.interrupt_mask = 0; // enable
    if(write_ioapic_redirect(redtbl_idx, entry) != 0) {
        logf("keyboard-init: write-ioapic-redirect failed\n");
    }
    asm("sti");

    //Let's do a keyboard read just to make sure it's empty
    in(KEYBOARD_ENCODER_PORT);
    // The following two bytes will read the scancode set used by the keyboard
    out(KEYBOARD_ENCODER_PORT, 0xF0);
    out(KEYBOARD_ENCODER_PORT, 0x00);
    current_modifiers = no_keys;
    uint8_t status_read = in(KEYBOARD_ENCODER_PORT);
    if(status_read == KEYBOARD_ACK_BYTE) {
        status_read = in(KEYBOARD_ENCODER_PORT);    
        logf("Found scancode set: 0x%x\n", status_read);
        scancode_set = status_read;
    } else {
        logf("Unable to read from keyboard\n");
    }
    out(0x64, PS2_READ_CONFIGURATION_COMMAND);
    status_read = in(PS2_STATUS_REGISTER);
    while((status_read & 2) != 0) {
        logf("Not ready yet... %x\n", status_read);
        status_read = in(PS2_STATUS_REGISTER);
    }
    uint8_t configuration_byte = in(PS2_DATA_REGISTER);    
    if((configuration_byte & (1 << 6)) != 0) {
        logf("Translation enabled\n");
        translation_enabled = true;
    } else {
        translation_enabled = false;        
    }
    // buf_position = 0;
    buf_write_head = 0;
    buf_read_head = 0;
    
}

uint8_t update_modifiers(key_modifiers modifier, bool is_pressed) {
    logf("modifier: %x\n", modifier);
    if ( is_pressed == true ) {
        current_modifiers |= modifier;
    } else {
        current_modifiers = current_modifiers  & ~(modifier);
    }
    
    return current_modifiers;
}

void handle_keyboard_interrupt() {
    
    uint8_t scancode = in(KEYBOARD_ENCODER_PORT);
    size_t buf_position;
  
    if(translation_enabled == true || scancode_set == 1) {
        buf_position = buf_write_head % MAX_KEYB_BUFFER_SIZE;
        uint8_t read_scancode = keyboard_buffer[buf_position].code = translate(scancode);        
        if( scancode != EXTENDED_PREFIX ) {
            keyboard_buffer[buf_position].code = read_scancode;
            keyboard_buffer[buf_position].modifiers = current_modifiers;
            if(scancode & KEY_RELEASE_MASK) {
                keyboard_buffer[buf_position].is_pressed = false;
            } else {
                keyboard_buffer[buf_position].is_pressed = true;
                // char read_char = kgetch(keyboard_buffer[buf_position]);
                // if (read_char != 0) {
                //     kprintf("%c",read_char);
                // }
                logf("+ Key is pressed pos %d: SC: %x - Code: %x - Mod: %x %c\n", buf_position, scancode, keyboard_buffer[buf_position].code, keyboard_buffer[buf_position].modifiers, kgetch(keyboard_buffer[buf_position]));
            }
            // buf_position = BUF_STEP(buf_position);
            buf_write_head++;
        } 
    }

    if(buf_write_head - buf_read_head > MAX_KEYB_BUFFER_SIZE) {
        buf_read_head = buf_write_head - MAX_KEYB_BUFFER_SIZE;
    } 
}

char kgetch(key_status char_item) {
    uint8_t char_code = char_item.code;
    if ( char_item.is_pressed == true) {
        if ( (keymap[char_code] >= 'a' && keymap[char_code] <= 'z') && (char_item.modifiers & 0b00000011) ) {
            const char offset = 'a' - 'A';
            return keymap[char_code] - offset;
        } else {
            if ( (keymap[char_code] >='0' && keymap[char_code] <= '9') && (char_item.modifiers & 0b00000011) ) {                
                return keymap[char_code] - ASCII_DIGIT_OFFSET;
            }

        }
        return keymap[char_item.code];
    }
    return 0;
}

key_codes translate(uint8_t scancode) {
    // TODO: check if key is released here.
    bool is_pressed = true;
    uint8_t read_scancode = scancode;
    if(scancode == EXTENDED_PREFIX) {
        extended_read = true;
        return scancode;
    }
    if (read_scancode & KEY_RELEASE_MASK) {
        is_pressed = false;
        read_scancode &= ~((uint8_t)KEY_RELEASE_MASK);
    }
    if(read_scancode < 0x60) {

        switch(read_scancode) {
        case LEFT_CTRL:
            update_modifiers(extended_read ? right_ctrl : left_ctrl, is_pressed);
            break;
        case LEFT_ALT:
            update_modifiers(extended_read ? right_alt : left_alt, is_pressed);
            break;
        case LEFT_GUI:
            update_modifiers(left_gui, is_pressed);
            read_scancode = 0;
            break;
        case RIGHT_GUI:
            update_modifiers(right_gui, is_pressed);
            read_scancode = 0;
            break;
        case LEFT_SHIFT:
            update_modifiers(left_shift, is_pressed);       
            break;
        case RIGHT_SHIFT:
            update_modifiers(right_shift, is_pressed);
            break;

        }
        extended_read = false;
        return scancode_mappings[read_scancode];
    }
    extended_read = false;
    return scancode_mappings[0];
}

key_status get_next_key() {
    if(buf_read_head < buf_write_head) {
        size_t buf_position = buf_read_head % MAX_KEYB_BUFFER_SIZE;
        buf_read_head++;

        return keyboard_buffer[buf_position];
    }
    key_status null_status;
    null_status.code = UNDEFINED;
    return null_status;
}