#ifndef FONTS_H
#define FONTS_H

#include <stdint.h>

// psf v1 magic: 0x036
#define PSF1_MAGIC     0x0436

// psf v2 magic: 0x864ab572
#define PSF2_MAGIC     0x864ab572

#define FONT_TYPE_PSF1 0
#define FONT_TYPE_PSF2 1

struct psf_font_v1_t {
    uint16_t magic;
    uint8_t mode;
    uint8_t charsize;
    // glyph_width:     8(1 byte)
    // glyph_height:    same as charsize
    // numglyphs:       512 if mode else 256
} __attribute__((packed));

struct psf_font_v2_t {
    uint32_t magic;             // 0x864ab572
    uint32_t version;           // 0
    uint32_t headersize;        // 32
    uint32_t flags;             // 0 -> no-unicode-table
    uint32_t numglyphs;
    uint32_t bytes_per_glyph;
    uint32_t glyph_height;      // in pixels
    uint32_t glyph_width;       // in pixels
} __attribute__((packed));

typedef struct {
    uint32_t type;
    uint32_t headersize;
    uint32_t num_glyphs;
    uint32_t glyph_size;
    uint32_t glyph_width;
    uint32_t glyph_height;

    uint64_t glyph_start;        // contains start address
} Font;

extern Font default_font;

void font_init();

/// @brief 
/// @param index code for the glyph, or glyph offset in the font structure
/// @return 
uint64_t get_glyph(uint32_t index, Font* font);

#endif