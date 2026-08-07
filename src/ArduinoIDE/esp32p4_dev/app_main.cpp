/**
 * @file app_main.cpp
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief アプリメイン
 * @version 0.1
 * @date 2026-08-06
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#include "app_main.h"
#include "common.h"
#include "pcb_def.h"

#if (PCB_TYPE == WT9932P4_TINY)
#include "app_neopixel.h"
#endif

#if (PCB_TYPE == JS_ESP32P4_M3_DEV)
#include "app_wifi.h"
#endif

// ---------------------------------------------------
// [DEBUG関連]
static void _dbg_pcb_info_print(void);

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

    Serial.begin(115200);
    delay(100);

#if (PCB_TYPE == WT9932P4_TINY)
    // RGBLED(Neopixel)初期化
    app_neopixel_init(RGBLED_PIN, RGBLED_NUM, RGBLED_MAX_BRIGHTNESS);
#endif

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

#if (PCB_TYPE == JS_ESP32P4_M3_DEV)
    Serial.printf("[DEBUG] WiFi Init()\n");
    app_wifi_init(MY_WIFI_SSID, MY_WIFI_PASSWORD);
#endif
}

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

static void _pcb_init(void)
{
    _dbg_pcb_info_print();
}

static void vTaskCore0(void *p_param)
{
    Serial.printf("[CPU Core 0] vTaskCore0\n");

    while (1)
    {
#if (PCB_TYPE == JS_ESP32P4_M3_DEV)
        // Serial.printf("[CPU Core 1] WiFi main()\n");
        app_wifi_main();
#endif
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void vTaskCore1(void *p_param)
{
    Serial.printf("[CPU Core 1] vTaskCore1\n");

    while (1)
    {
#if (PCB_TYPE == WT9932P4_TINY)
        app_neopixel_rgb_illumination(0);
#endif
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

#ifdef DEBUG_TASK
static void vTaskDebug(void *p_param)
{
    static uint8_t s_cpu_core = xPortGetCoreID();

    Serial.printf("[CPU Core %d] vTaskDebug\n", s_cpu_core);

    while (1)
    {
        _dbg_pcb_info_print();
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}
#endif // DEBUG_TASK

static void _freertos_init(void)
{
#if 1
    // [RTOSタスク @CPU Core 0]
    xTaskCreatePinnedToCore(vTaskCore0,        // コールバック関数ポインタ
                            "vTaskCore0",      // タスク名
                            16384,              // スタック
                            NULL,              // パラメータ
                            6,                 // 優先度(0～7、7が最優先)
                            &s_xTaskCore0,     // ハンドル
                            DRV_CPU_CORE       // CPUのコア選択
                            );
#endif

#if 0
    // [RTOSタスク @CPU Core 1]
    xTaskCreatePinnedToCore(vTaskCore1,        // コールバック関数ポインタ
                            "vTaskCore1",      // タスク名
                            16384,              // スタック
                            NULL,              // パラメータ
                            4,                 // 優先度(0～7、7が最優先)
                            &s_xTaskCore1,     // ハンドル
                            APP_PROC_CORE      // CPUのコア選択
                            );
#endif

#ifdef DEBUG_TASK
    // [デバッグ用のタスク @CPU Core 1]
    xTaskCreatePinnedToCore(vTaskDebug,        // コールバック関数ポインタ
                            "vTaskDebug",      // タスク名
                            16384,              // スタック
                            NULL,              // パラメータ
                            1,                 // 優先度(0～7、7が最優先)
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