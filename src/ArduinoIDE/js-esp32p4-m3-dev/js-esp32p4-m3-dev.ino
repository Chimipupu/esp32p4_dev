/**
 * @file js-esp32p4-m3-dev.ino
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief ArduinoIDE用ファイル
 * @version 0.1
 * @date 2025-11-20
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */

#include <stdint.h>
#include <string.h>
#include <esp_system.h>

uint32_t g_cpu_freq_MHz = 0;
uint32_t  g_psram_size = 0;

void setup(void)
{
    Serial.begin(115200);
    delay(1000);

    // [基板名]
    // (例)　PCB: JS-ESP32P4-M3-DEV
    Serial.printf("PCB: JS-ESP32P4-M3-DEV\n");

    // [ESPIDFのバージョン取得]
    // (例)　ESP-IDF Version: v5.5.1-710-g8410210c9a
    Serial.printf("ESP-IDF Version: %s\n", esp_get_idf_version());

    // [CPUのクロック周波数取得]
    // (例)　CPU Clock: 360 MHz
    g_cpu_freq_MHz = getCpuFrequencyMhz();
    Serial.printf("CPU Clock: %u MHz\n", g_cpu_freq_MHz);

    // [PSRAMのサイズ取得]
    // (例)　PSRAM Size: 32 MB
    uint32_t g_psram_size = ESP.getPsramSize();
    if (g_psram_size > 0) {
        Serial.printf("PSRAM Size: %u MB\n", g_psram_size / (1024 * 1024));
    } else {
        Serial.printf("PSRAM: Not available\n");
    }
}

void loop(void)
{
    // TODO:
}