/**
 * @file app_main.cpp
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief アプリメイン
 * @version 0.1
 * @date 2026-08-23
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#include "app_main.h"
#include "app_freertos.h"

#ifdef RGBLED_USE
#include "app_neopixel.h"
#endif

// ---------------------------------------------------
// [DEBUG関連]
static void _dbg_pcb_info_print(void);

// ---------------------------------------------------
#if (PCB_TYPE == JS_ESP32P4_M3_DEV)
const char *p_pcb_name_str = "JS-ESP32P4-M3-DEV";
#elif (PCB_TYPE == WT9932P4_TINY)
const char *p_pcb_name_str = "WT9932P4-Tiny";
#endif

// ---------------------------------------------------
// [Static]
static char *p_chip_model_str = NULL;
static float s_chip_rev = 0;
static uint32_t s_cpu_freq_MHz = 0;
static uint8_t s_cpu_core_num = 0;
static uint8_t s_flash_size_mega_byte = 0;
static uint8_t s_psram_size_mega_byte = 0;
static char *p_esp_idf_ver_str = NULL;

static void _mcu_init(void);
static void _pcb_init(void);

// ---------------------------------------------------
// [Static関数]

static void _mcu_init(void)
{
    uint32_t tmp_u32;

    Serial.begin(115200);
    // delay(100);

#ifdef RGBLED_USE
    // RGBLED(Neopixel)初期化
    app_neopixel_init(RGBLED_PIN, RGBLED_NUM, RGBLED_MAX_BRIGHTNESS);
#endif

    // ESP32のチップの種類とリビジョンを取得
    p_chip_model_str = (char *)ESP.getChipModel();
    tmp_u32 = ESP.getChipRevision();
    s_chip_rev = (float)tmp_u32 / 100.0f;

    // CPUのクロック周波数とコア数を取得
    s_cpu_freq_MHz = getCpuFrequencyMhz();
    s_cpu_core_num = ESP.getChipCores();

    // Flashのサイズを取得
    tmp_u32 = ESP.getFlashChipSize();
    s_flash_size_mega_byte = (uint8_t)(tmp_u32 / (1024 * 1024));

    // PSRAMのサイズを取得
    tmp_u32 = ESP.getPsramSize();
    s_psram_size_mega_byte = (uint8_t)(tmp_u32 / (1024 * 1024));

    // ESPIDFのバージョン取得
    p_esp_idf_ver_str = (char *)esp_get_idf_version();
}

static void _dbg_pcb_info_print(void)
{
    Serial.printf("-------------------------------\n");
    Serial.printf("PCB: %s\n", p_pcb_name_str);
    Serial.printf("ESP32 Chip Model: %s\n", p_chip_model_str);
    Serial.printf("ESP32 Chip Rev: v%.01f\n", s_chip_rev);
    Serial.printf("CPU Clock: %lu MHz\n", s_cpu_freq_MHz);
    Serial.printf("CPU Core: x%d\n", s_cpu_core_num);
    Serial.printf("Flash Size: %d MB\n", s_flash_size_mega_byte);
    if (s_psram_size_mega_byte > 0) {
        Serial.printf("PSRAM Size: %d MB\n", s_psram_size_mega_byte);
    } else {
        Serial.printf("PSRAM: Not available\n");
    }
    Serial.printf("ESP-IDF Version: %s\n", p_esp_idf_ver_str);
    Serial.printf("-------------------------------\n");
}

static void _pcb_init(void)
{
    _dbg_pcb_info_print();
}
// ---------------------------------------------------
// [API]

/**
 * @brief アプリメイン初期化
 */
void app_main_init(void)
{
    _mcu_init();         // マイコン初期化
    _pcb_init();         // 基板初期化
    app_freertos_init(); // FreeRTOS 初期化
}

/**
 * @brief アプリメイン
 */
void app_main(void)
{
    // NOTE: loopタスクはいらない
    Serial.printf("loop Task is Suspend !!!\n");
    vTaskSuspend(NULL);
}