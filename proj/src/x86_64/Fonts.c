#include "Fonts.h"
#include "VMM.h"
#include "MEM.h"

#include "Logging.h"

extern char* _binary_proj_res_fonts_default_font_psf_start;
extern char* _binary_proj_res_fonts_default_font_psf_end;
extern char* _binary_proj_res_fonts_default_font_psf_size;

Font default_font;

void font_init() {
    // memory map header
    identity_map_phys_address((void*)_binary_proj_res_fonts_default_font_psf_start, PRESENT_BIT);

    char* font_start = _binary_proj_res_fonts_default_font_psf_start + 64; // offet of ELF header
    if(*((uint16_t*)font_start) == PSF1_MAGIC)
    {
        logf("\nVersion: PSF1\n");
        struct psf_font_v1_t* psf1 = (struct psf_font_v1_t*)font_start;

        default_font.type       = FONT_TYPE_PSF1;
        default_font.headersize = sizeof(struct psf_font_v1_t);
        
        if(psf1->mode)  default_font.num_glyphs = 512;
        else            default_font.num_glyphs = 256;

        default_font.glyph_width    = 8; // in bits
        default_font.glyph_height   = psf1->charsize;
        default_font.glyph_size     = default_font.glyph_width * default_font.glyph_height;
    }
    else if(*((uint32_t*)font_start) == PSF2_MAGIC)
    {
        logf("\nVersion: PSF2\n");
        struct psf_font_v2_t* psf2 = (struct psf_font_v2_t*)font_start;

        default_font.type           = FONT_TYPE_PSF2;
        default_font.headersize     = psf2->headersize;
        default_font.num_glyphs     = psf2->numglyphs;
        default_font.glyph_width    = psf2->glyph_width;
        default_font.glyph_height   = psf2->glyph_height;
        default_font.glyph_size     = psf2->glyph_width * psf2->glyph_height;
    }
    else {
        logf("\nVersion: OTHER\n");
        logf("Magic: %ul\n", *(uint32_t*)font_start);
    }
    default_font.glyph_start = (uint64_t)((uint8_t*)font_start + default_font.headersize);
    
    // memory-map font
    size_t font_memory = default_font.headersize + default_font.glyph_size * default_font.num_glyphs;
    for(size_t i=0; i < font_memory; i++) {
        identity_map_phys_address((void*)(default_font.glyph_start + i), PRESENT_BIT);
    }
}

uint64_t get_glyph(uint32_t index, Font* font) {
    if(index > font->num_glyphs) index = 0;
    return (uint64_t)(font->glyph_start + font->glyph_size * index);
}