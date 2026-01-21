#ifndef VIDEO_VESA_H
#define VIDEO_VESA_H

#include <stdint.h>

// Helpers expostos pelo driver VESA para rotinas de desenho rápidas.
// (Somente válidos quando o driver ativo é o VESA framebuffer.)

volatile uint32_t* vesa_framebuffer_ptr(void);
uint32_t vesa_pitch_pixels(void);
void vesa_fill_rect_fast(int x, int y, int w, int h, uint32_t color);

#endif
