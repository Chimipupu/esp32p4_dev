/**
 * @file app_wifi.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief WiFiアプリ
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
// void app_wifi_scan(void);
void app_wifi_init(const char *p_ssid, const char *p_password);
void app_wifi_main(void);

#endif // APP_WIFI_H