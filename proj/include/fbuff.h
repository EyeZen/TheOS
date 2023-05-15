#ifndef _FBUFF_H
#define _FBUFF_H

#include <stdint.h>
#include <stddef.h>
#include "multiboot.h"
#include "Fonts.h"

typedef uint32_t Pixel;
#define PIXEL_SIZE sizeof(Pixel)


typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t* addr;
    uint8_t bpp;
    uint32_t buffer_size;
} Framebuffer;
extern Framebuffer framebuffer;

#define ALPHA_MASK 0xffffff00

#define RED     0x00ff0000
#define GREEN   0x0000ff00
#define BLUE    0x000000ff
#define GRAY    0x00fefefe
#define WHITE   0x00ffffff
#define BLACK   0x00000000


void fb_init(struct multiboot_tag_framebuffer* mb_framebuffer);
uint32_t fb_color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
void fb_put_pixel(uint32_t x, uint32_t y, uint32_t color);
void fb_draw_logo();
void fb_putchar(char symbol, size_t cx, size_t cy, uint32_t fg, uint32_t bg, Font* font);
void fb_printStr(const char *string, size_t cx, size_t cy, uint32_t fg, uint32_t bg, Font* font);

#endif