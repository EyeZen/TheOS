#include "fbuff.h"
#include "VMM.h"
#include "MEM.h"
#include "Fonts.h"
#include "twitter_logo.h"
#include "Logging.h"
#include "utils.h"

Framebuffer framebuffer;

void fb_init(struct multiboot_tag_framebuffer* mb_framebuffer) {
    framebuffer.addr = (uint32_t*)(mb_framebuffer->common.framebuffer_addr);
    framebuffer.width = mb_framebuffer->common.framebuffer_width;
    framebuffer.height = mb_framebuffer->common.framebuffer_height;
    framebuffer.pitch = mb_framebuffer->common.framebuffer_pitch;
    framebuffer.bpp = mb_framebuffer->common.framebuffer_bpp;
    // in bytes
    framebuffer.buffer_size = framebuffer.width * framebuffer.height * (framebuffer.bpp / 8);

    for(uint32_t i=0; i < framebuffer.buffer_size; i++) {
        identity_map_phys_address((void*)(framebuffer.addr + i), (PRESENT_BIT | WRITE_BIT));
    }

    // font_init();
}

uint32_t fb_color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) {
    uint32_t color  = ((uint32_t)red << 16)
                    | ((uint32_t)green << 8) 
                    | ((uint32_t)blue << 0) 
                    | ((uint32_t)alpha << 24);

    return color;
}

void fb_put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    uint32_t col = y * framebuffer.pitch;
    uint32_t row = x * (framebuffer.bpp / 8);
    *(uint32_t*)((uint64_t)framebuffer.addr + row + col) = color;
}

void fb_clear(uint32_t color) {
    for(uint32_t i=0; i < framebuffer.buffer_size; i++) {
        *(uint32_t*)((uint64_t)framebuffer.addr + i) = color;
    }
}

void fb_draw_logo() {
    char pixel[4];
    for(uint32_t y = 0; y < twitter_logo_height; y++) {
        for(uint32_t x = 0; x < twitter_logo_width; x++) {
            HEADER_PIXEL(header_data, pixel);
            pixel[3] = 0;
            fb_put_pixel(x, y, fb_color(pixel[0], pixel[1], pixel[2], pixel[2]));
        }
    }
}

/// @brief 
/// @param symbol character to print 
/// @param cx position in character units
/// @param cy position in character units
/// @param fg foreground color
/// @param bg background color
void fb_putchar(char symbol, size_t cx, size_t cy, uint32_t fg, uint32_t bg, Font* font)
{
    size_t off_x = cx * font->glyph_width * PIXEL_SIZE;
    size_t off_y = cy * font->glyph_height * framebuffer.pitch;

    uint8_t* glyph = (uint8_t*)get_glyph(symbol, font);

    size_t glyph_bytes_per_row = (font->glyph_width + 7) / 8;
    size_t buffer_offset = off_x + off_y;

    uint32_t glyph_offset;

    for(uint32_t y = 0; y < font->glyph_height; y++) {
        glyph_offset = buffer_offset;
        // logf("%x\n", glyph[0]);
        for(uint32_t x = 0; x < font->glyph_width; x++) {
            // (current-byte)glyph & (0b10000000 >> (x % 7)), (if, in current byte, nth byte from end is set, then its fg else bg pixel)
            *(Pixel*)((uint64_t)framebuffer.addr + glyph_offset) = glyph[x/8] & (0x80 >> (x & 7)) ? fg : bg;
            glyph_offset += sizeof(framebuffer.bpp / 8);
        }
        glyph += glyph_bytes_per_row;
        buffer_offset += framebuffer.pitch;
    }
}

void fb_printStr(const char *string, size_t cx, size_t cy, uint32_t fg, uint32_t bg, Font* font) {
    while (*string != '\0'){
        if (*string == '\n'){
            cx=0;
            cy++;
        } else {
            fb_putchar(*string, cx, cy, fg, bg, font);
            cx++;
        }
        string++;
    }
}