/**
 * @file app_wifi.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief WiFiアプリ (for ESP32-P4)
 * @note ESP32-P4とSDIOで接続されてるESP32-C6専用の実装
 * @version 0.1
 * @date 2026-08-06
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#ifndef APP_WIFI_H
#define APP_WIFI_H

#include <Arduino.h>
#include "WiFi.h"

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

// -----------------------------------------------------------
#define MY_WIFI_SSID       "B4865595449D-2G" // WiFiのSSID
#define MY_WIFI_PASSWORD   "55351108940066"  // WiFiのパスワード
#if !defined(MY_WIFI_SSID) || !defined(MY_WIFI_PASSWORD)
#error "[ERROR] Please define Your Wifi SSID and Password in app_wifi.h"
#endif

#define NTP_TIMEZONE_JST    (9 * 3600) // 日本標準時のタイムゾーン（UTC+9）
// -----------------------------------------------------------
#if (PCB_TYPE == JS_ESP32P4_M3_DEV)
extern xTaskHandle g_xTaskWiFi;
void vTaskWiFi(void *p_param);
#endif

#endif // APP_WIFI_H