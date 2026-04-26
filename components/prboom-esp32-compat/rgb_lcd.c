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
 */

#include "rgb_lcd.h"

#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_ops.h"
#include "esp_heap_caps.h"
#include "driver/gpio.h"
#include "esp_log.h"
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

static esp_lcd_panel_handle_t s_panel = NULL;
static uint16_t              *s_fb    = NULL;

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
        /* B[0..4], G[0..5], R[0..4]  — LSB first within each channel */
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

    ESP_LOGI(TAG, "RGB panel ready, fb=%p", (void *)s_fb);
}

uint16_t *rgb_lcd_get_frame_buffer(void)
{
    return s_fb;
}
