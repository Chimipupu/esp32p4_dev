/**
 * @file pcb_def.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief 基板別定義
 * @version 0.1
 * @date 2026-08-21
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#ifndef PCB_DEF_H
#define PCB_DEF_H

// ---------------------------------------------------
#define WT9932P4_TINY        0x00
#define JS_ESP32P4_M3_DEV    0x01

#define PCB_TYPE             WT9932P4_TINY
// #define PCB_TYPE             JS_ESP32P4_M3_DEV

/**
 * @brief WT9932P4_TINY
 * @note Flash: 16MB, PSRAM: 32MB
 * @note WiFi/Bluetooth: (N/A)
 */
#if (PCB_TYPE == WT9932P4_TINY)
#define RGBLED_USE
#define RGBLED_PIN               51  // RGBLED(Neopixel)のデータピンとの接続GPIO: IO51
#define RGBLED_NUM               1   // RGBLEDの数
#define RGBLED_MAX_BRIGHTNESS    16  // RGBLEDの最大輝度
#define RGBLED_COLOR_ON_TIMER    100 // RGBLEDの1色の表示時間
#endif // (PCB_TYPE == WT9932P4_TINY)

/**
 * @brief JS_ESP32P4_M3_DEV
 * @note Flash: 16MB, PSRAM: 32MB
 * @note WiFi/Bluetooth: ESP32-C6
 */
#if (PCB_TYPE == JS_ESP32P4_M3_DEV)
#define WIFI_USE
#define RGBLED_NONE
#define WIFI_BLUETOOTH_COPROCESSOR_ESP32C6
#endif // (PCB_TYPE == JS_ESP32P4_M3_DEV)

// ---------------------------------------------------

#endif // PCB_DEF_H
