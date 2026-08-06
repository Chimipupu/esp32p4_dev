/**
 * @file app_main.cpp
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief アプリメイン
 * @version 0.1
 * @date 2026-03-28
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#include "app_main.h"
#include "common.h"
#include "pcb_def.h"

// ---------------------------------------------------
// [DEBUG関連]
#ifdef DEBUG_APP
static void _dbg_pcb_info_print(void);
#endif // DEBUG_APP

// ---------------------------------------------------
// [グローバル]
#if (PCB_TYPE == JS_ESP32P4_M3_DEV)
const char *p_pcb_name_str = "JS-ESP32P4-M3-DEV";
#elif (PCB_TYPE == WT9932P4_TINY)
const char *p_pcb_name_str = "WT9932P4-Tiny";
#endif

const char *p_chip_model_str = NULL;
float g_chip_rev = 0;
uint32_t g_cpu_freq_MHz = 0;
uint8_t g_cpu_core_num = 0;
uint8_t g_flash_size_mega_byte = 0;
uint8_t g_psram_size_mega_byte = 0;
const char *p_esp_idf_ver_str = NULL;
// ---------------------------------------------------
// [Static]

// FreeRTOS関連
static xTaskHandle s_xTaskCore0;
static xTaskHandle s_xTaskCore1;
#ifdef DEBUG_TASK
static xTaskHandle s_xTaskDebug;
static void vTaskDebug(void *p_param);
#endif // DEBUG_TASK
static void vTaskCore0(void *p_param);
static void vTaskCore1(void *p_param);

static void _freertos_init(void);
static void _mcu_init(void);
static void _pcb_init(void);
// ---------------------------------------------------
// [Static関数]

static void _mcu_init(void)
{
    uint32_t tmp_u32;

    // ESP32のチップの種類とリビジョンを取得
    p_chip_model_str = ESP.getChipModel();
    tmp_u32 = ESP.getChipRevision();
    g_chip_rev = (float)tmp_u32 / 100.0f;

    // CPUのクロック周波数とコア数を取得
    g_cpu_freq_MHz = getCpuFrequencyMhz();
    g_cpu_core_num = ESP.getChipCores();

    // Flashのサイズを取得
    tmp_u32 = ESP.getFlashChipSize();
    g_flash_size_mega_byte = (uint8_t)(tmp_u32 / (1024 * 1024));

    // PSRAMのサイズを取得
    tmp_u32 = ESP.getPsramSize();
    g_psram_size_mega_byte = (uint8_t)(tmp_u32 / (1024 * 1024));

    // ESPIDFのバージョン取得
    p_esp_idf_ver_str = esp_get_idf_version();
}

#ifdef DEBUG_APP
static void _dbg_pcb_info_print(void)
{
    Serial.printf("-------------------------------\n");
    Serial.printf("PCB: %s\n", p_pcb_name_str);
    Serial.printf("ESP32 Chip Model: %s\n", p_chip_model_str);
    Serial.printf("ESP32 Chip Rev: v%.01f\n", g_chip_rev);
    Serial.printf("CPU Clock: %lu MHz\n", g_cpu_freq_MHz);
    Serial.printf("CPU Core: x%d\n", g_cpu_core_num);
    Serial.printf("Flash Size: %d MB\n", g_flash_size_mega_byte);
    if (g_psram_size_mega_byte > 0) {
        Serial.printf("PSRAM Size: %d MB\n", g_psram_size_mega_byte);
    } else {
        Serial.printf("PSRAM: Not available\n");
    }
    Serial.printf("ESP-IDF Version: %s\n", p_esp_idf_ver_str);
    Serial.printf("-------------------------------\n");
}
#endif // DEBUG_APP

static void _pcb_init(void)
{
#ifdef DEBUG_APP
    _dbg_pcb_info_print();
#endif
}

static void vTaskCore0(void *p_param)
{
    // static uint8_t s_cpu_core_num  = xPortGetCoreID();

    while (1)
    {
        Serial.printf("[CPU Core %d] vTaskCore0\n", DRV_CPU_CORE);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void vTaskCore1(void *p_param)
{
    // static uint8_t s_cpu_core_num  = xPortGetCoreID();

    while (1)
    {
        Serial.printf("[CPU Core %d] vTaskCore1\n", APP_PROC_CORE);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

#ifdef DEBUG_TASK
static void vTaskDebug(void *p_param)
{
    // static uint8_t s_cpu_core_num  = xPortGetCoreID();

    while (1)
    {
        Serial.printf("[CPU Core %d] vTaskDebug\n", APP_PROC_CORE);
#ifdef DEBUG_APP
        _dbg_pcb_info_print();
#endif
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
#endif // DEBUG_TASK

static void _freertos_init(void)
{
    // [RTOSタスク @CPU Core 0]
    xTaskCreatePinnedToCore(vTaskCore0,        // コールバック関数ポインタ
                            "vTaskCore0",      // タスク名
                            2048,              // スタック
                            NULL,              // パラメータ
                            7,                 // 優先度(0～7、7が最優先)
                            &s_xTaskCore0,     // ハンドル
                            DRV_CPU_CORE       // CPUのコア選択
                            );

    // [RTOSタスク @CPU Core 1]
    xTaskCreatePinnedToCore(vTaskCore1,        // コールバック関数ポインタ
                            "vTaskCore1",      // タスク名
                            2048,              // スタック
                            NULL,              // パラメータ
                            5,                 // 優先度(0～7、7が最優先)
                            &s_xTaskCore1,     // ハンドル
                            APP_PROC_CORE      // CPUのコア選択
                            );

#ifdef DEBUG_TASK
    xTaskCreatePinnedToCore(vTaskDebug,        // コールバック関数ポインタ
                            "vTaskDebug",      // タスク名
                            4096,              // スタック
                            NULL,              // パラメータ
                            3,                 // 優先度(0～7、7が最優先)
                            &s_xTaskDebug,     // ハンドル
                            APP_PROC_CORE      // CPUのコア選択
                            );
#endif // DEBUG_TASK
}

// ---------------------------------------------------
// [API]

/**
 * @brief アプリメイン初期化
 */
void app_main_init(void)
{
    _mcu_init();      // マイコン初期化
    _pcb_init();      // 基板初期化
    _freertos_init(); // FreeRTOS 初期化
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