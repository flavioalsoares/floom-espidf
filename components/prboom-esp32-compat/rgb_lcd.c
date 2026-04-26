/*
 * RGB parallel LCD driver for ESP32-8048S070 (800x480, EK9716 controller)
 * Uses ESP-IDF esp_lcd_rgb_panel API.
 *
 * Pin assignments (from board schematic):
 *   PCLK=42, DE=41, VSYNC=40, HSYNC=39, BK_LIGHT=2
 *   R[0..4] = 14,21,47,48,45
 *   G[0..5] =  9,46, 3, 8,16, 1
 *   B[0..4] = 15, 7, 6, 5, 4
 *
 * Timing (EK9716 / 16 MHz):
 *   H: front_porch=210, pulse_width=30, back_porch=16
 *   V: front_porch=22,  pulse_width=13, back_porch=10
 *
 * Blit pipeline: a dedicated display task pinned to core 0 owns the
 * 8bpp -> RGB565 + 2x upscale work. The Doom thread (core 1) only kicks
 * the task and waits at start of next frame, so game logic (TryRunTics)
 * runs in parallel with the blit.
 */

#include "rgb_lcd.h"

#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_ops.h"
#include "esp_heap_caps.h"
#include "esp_attr.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <string.h>

#define TAG "rgb_lcd"

#define LCD_H_RES        800
#define LCD_V_RES        480
#define LCD_PCLK_HZ      (16 * 1000 * 1000)

#define LCD_GPIO_PCLK    42
#define LCD_GPIO_DE      41
#define LCD_GPIO_VSYNC   40
#define LCD_GPIO_HSYNC   39
#define LCD_GPIO_BK      2

#define DOOM_W   320
#define DOOM_H   240
#define X_OFF    ((LCD_H_RES - DOOM_W * 2) / 2)   /* 80 */

static esp_lcd_panel_handle_t s_panel = NULL;
static uint16_t              *s_fb    = NULL;

/* --- async blit state ---------------------------------------------------- */
static SemaphoreHandle_t        s_kick_sem    = NULL;   /* game -> task: go */
static SemaphoreHandle_t        s_done_sem    = NULL;   /* task -> game: done */
static volatile bool            s_blit_pending = false;
static const uint8_t * volatile s_blit_src    = NULL;
static DRAM_ATTR uint16_t       s_pal_snap[256];        /* palette snapshot */

/* 640-pixel line buffer in DRAM, owned by the display task only. */
static DRAM_ATTR uint32_t s_linebuf[DOOM_W];   /* 320 x uint32 = 640 px */

static void IRAM_ATTR display_task(void *arg)
{
    while (1) {
        xSemaphoreTake(s_kick_sem, portMAX_DELAY);

        const uint8_t *src = s_blit_src;
        uint16_t      *fb  = s_fb;

        if (src && fb) {
            for (int y = 0; y < DOOM_H; y++) {
                const uint8_t *srow = src + y * DOOM_W;

                /* palette lookup + 2x horiz: one uint32 write per source pixel */
                for (int x = 0; x < DOOM_W; x++) {
                    uint32_t c = s_pal_snap[srow[x]];
                    s_linebuf[x] = (c << 16) | c;
                }

                /* two sequential memcpy to PSRAM (burst-friendly) */
                uint16_t *row0 = fb + (y * 2 + 0) * LCD_H_RES + X_OFF;
                uint16_t *row1 = fb + (y * 2 + 1) * LCD_H_RES + X_OFF;
                memcpy(row0, s_linebuf, DOOM_W * 2 * sizeof(uint16_t));
                memcpy(row1, s_linebuf, DOOM_W * 2 * sizeof(uint16_t));
            }
        }

        xSemaphoreGive(s_done_sem);
    }
}

void rgb_lcd_init(void)
{
    if (s_panel) return;  /* already initialised */

    /* backlight on */
    gpio_config_t bk = {
        .pin_bit_mask = 1ULL << LCD_GPIO_BK,
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&bk);
    gpio_set_level(LCD_GPIO_BK, 1);

    esp_lcd_rgb_panel_config_t cfg = {
        .clk_src    = LCD_CLK_SRC_DEFAULT,
        .timings = {
            .pclk_hz         = LCD_PCLK_HZ,
            .h_res           = LCD_H_RES,
            .v_res           = LCD_V_RES,
            .hsync_back_porch  = 16,
            .hsync_front_porch = 210,
            .hsync_pulse_width = 30,
            .vsync_back_porch  = 10,
            .vsync_front_porch = 22,
            .vsync_pulse_width = 13,
            .flags.pclk_active_neg = false,
        },
        .data_width  = 16,
        .num_fbs     = 1,
        /* bounce buffer keeps DMA in fast SRAM while FB lives in PSRAM */
        .bounce_buffer_size_px = 10 * LCD_H_RES,
        .hsync_gpio_num = LCD_GPIO_HSYNC,
        .vsync_gpio_num = LCD_GPIO_VSYNC,
        .de_gpio_num    = LCD_GPIO_DE,
        .pclk_gpio_num  = LCD_GPIO_PCLK,
        .disp_gpio_num  = GPIO_NUM_NC,
        /* B[0..4], G[0..5], R[0..4]  - LSB first within each channel */
        .data_gpio_nums = { 15, 7, 6, 5, 4,       /* B0-B4 */
                             9,46, 3, 8,16, 1,     /* G0-G5 */
                            14,21,47,48,45 },       /* R0-R4 */
        .flags.fb_in_psram = true,
    };

    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&cfg, &s_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(s_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(s_panel));

    ESP_ERROR_CHECK(
        esp_lcd_rgb_panel_get_frame_buffer(s_panel, 1, (void **)&s_fb));

    /* paint framebuffer black once */
    memset(s_fb, 0, (size_t)LCD_H_RES * LCD_V_RES * sizeof(uint16_t));

    /* spin up the display task on core 0 (game runs on core 1) */
    s_kick_sem = xSemaphoreCreateBinary();
    s_done_sem = xSemaphoreCreateBinary();
    xTaskCreatePinnedToCore(display_task, "rgb_lcd_disp",
                            4096, NULL, 5, NULL, 0);

    ESP_LOGI(TAG, "RGB panel ready, fb=%p, display task on core 0",
             (void *)s_fb);
}

uint16_t *rgb_lcd_get_frame_buffer(void)
{
    return s_fb;
}

void rgb_lcd_blit_kick(const uint8_t *src8bpp_320x240, const uint16_t *pal256)
{
    if (!s_kick_sem) return;
    /* snapshot palette so the game can mutate lcdpal without racing the task */
    if (pal256) memcpy(s_pal_snap, pal256, sizeof(s_pal_snap));
    s_blit_src     = src8bpp_320x240;
    s_blit_pending = true;
    xSemaphoreGive(s_kick_sem);
}

void rgb_lcd_blit_wait(void)
{
    if (!s_blit_pending) return;   /* no kick outstanding -> nothing to wait for */
    xSemaphoreTake(s_done_sem, portMAX_DELAY);
    s_blit_pending = false;
}
