/**
 * @file app_neopixel.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief NeoPixelアプリ
 * @version 0.1
 * @date 2026-08-06
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */
#ifndef APP_NEOPIXEL_HPP
#define APP_NEOPIXEL_HPP

#include "common.h"
#include <Adafruit_NeoPixel.h>

// ---------------------------------------------------
// [コンパイルスイッチ]
#define DEBUG_RGB_LED

// ------------------------------------------------------------------------------
typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} led_color_t;

typedef struct {
    led_color_t rgb;
    const char *p_color_str;
} led_color_data_t;

// ------------------------------------------------------------------------------
void app_neopixel_init(uint8_t led_pin, uint8_t led_num, uint8_t brightness);
void app_neopixel_set_rgb(uint8_t led_no, led_color_t *p_rgb);
void app_neopixel_rgb_illumination(uint8_t led_no);
void app_neopixel_set_brightness(uint8_t brightness);

#endif /* APP_NEOPIXEL_HPP */