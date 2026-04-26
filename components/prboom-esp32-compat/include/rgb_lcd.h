#pragma once
#include <stdint.h>

void rgb_lcd_init(void);
uint16_t *rgb_lcd_get_frame_buffer(void);

/* Async blit pipeline (display task on core 0).
 *
 * Doom's I_FinishUpdate calls rgb_lcd_blit_kick() to hand a 320x240 8bpp
 * frame + palette to the display task and returns immediately. The display
 * task does palette lookup + 2x scale + write into the RGB framebuffer.
 *
 * Doom's I_StartFrame calls rgb_lcd_blit_wait() before the next render so
 * the source frame (screens[0]) is not modified while the display task is
 * still reading it.
 */
void rgb_lcd_blit_kick(const uint8_t *src8bpp_320x240, const uint16_t *pal256);
void rgb_lcd_blit_wait(void);
