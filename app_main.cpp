/**
 * @file app_main.cpp
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief アプリメイン
 * @version 0.1
 * @date 2026-08-21
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#include "app_main.h"
#include "common.h"
#include "pcb_def.h"

#ifdef RGBLED_USE
#include "app_neopixel.h"
static led_color_t s_request_rgb_led_val = {.rgb = 0x000000};
static led_color_t s_local_rgb_led_val = {.rgb = 0x000000};
#endif

#ifdef WIFI_USE
#include "app_wifi.h"
static const char *g_wifi_ssid = MY_WIFI_SSID;
static const char *g_wifi_password = MY_WIFI_PASSWORD;
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
static void vTaskCore0(void *p_param);
static void vTaskCore1(void *p_param);
QueueHandle_t g_queue_handle_core2core = NULL;

#ifdef DEBUG_TASK
static xTaskHandle s_xTaskDebug;
static void vTaskDebug(void *p_param);
#endif // DEBUG_TASK

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

#ifdef RGBLED_USE
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
    BaseType_t que_status;
    static uint8_t s_core_num = xPortGetCoreID();
    Serial.printf("[CPU Core %d] vTaskCore0\n", s_core_num);

    while (1)
    {
        que_status = xQueueReceive(g_queue_handle_core2core,
                                    &s_local_rgb_led_val,
                                    portMAX_DELAY);
        if(que_status == pdPASS)
        {
#ifdef RGBLED_USE
            app_neopixel_set_rgb(0, &s_local_rgb_led_val);
            Serial.printf("[CPU Core %d] RGB_LED: Set Color = (0x%02X, 0x%02X, 0x%02X)\n", s_core_num,
                            s_local_rgb_led_val.para.red,
                            s_local_rgb_led_val.para.green,
                            s_local_rgb_led_val.para.blue);
#endif
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

static void vTaskCore1(void *p_param)
{
    BaseType_t que_status;
    static uint8_t s_tbl_idx = 0;
    static uint8_t s_core_num = xPortGetCoreID();
    Serial.printf("[CPU Core %d] vTaskCore1\n", s_core_num);

    while (1)
    {
#ifdef RGBLED_USE
        que_status = xQueueSend(g_queue_handle_core2core,
                                &s_request_rgb_led_val,
                                portMAX_DELAY);

        // キューの送信完了を受けて次のデータを用意しておく
        if(que_status == pdPASS)
        {
            Serial.printf("[CPU Core %d] RGB_LED: Request Color = (0x%02X, 0x%02X, 0x%02X)\n", s_core_num,
                            g_led_color_tbl[s_tbl_idx].rgb.para.red,
                            g_led_color_tbl[s_tbl_idx].rgb.para.green,
                            g_led_color_tbl[s_tbl_idx].rgb.para.blue);

            s_request_rgb_led_val.rgb = g_led_color_tbl[s_tbl_idx].rgb.rgb;
            s_tbl_idx = (s_tbl_idx + 1) % (RGBLED_COLOR_TBL_SIZE - 1);
        }
#endif
        vTaskDelay(500 / portTICK_PERIOD_MS);
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
    // キューの作成
#ifdef RGBLED_USE
    g_queue_handle_core2core = xQueueCreate(8, sizeof(led_color_t));
#endif

    // [RTOSタスク @CPU Core 0]
    xTaskCreatePinnedToCore(vTaskCore0,        // コールバック関数ポインタ
                            "vTaskCore0",      // タスク名
                            4096,              // スタック
                            NULL,              // パラメータ
                            3,                 // 優先度(0～7、7が最優先)
                            &s_xTaskCore0,     // ハンドル
                            DRV_CPU_CORE       // CPUのコア選択
                            );

#ifdef WIFI_USE
    // [WiFiタスク @CPU Core 0]
    xTaskCreatePinnedToCore(vTaskWiFi,         // コールバック関数ポインタ
                            "vTaskWiFi",       // タスク名
                            8192,              // スタック
                            NULL,              // パラメータ
                            6,                 // 優先度(0～7、7が最優先)
                            &g_xTaskWiFi,      // ハンドル
                            DRV_CPU_CORE       // CPUのコア選択
                            );
#endif // WIFI_USE

    // [RTOSタスク @CPU Core 1]
    xTaskCreatePinnedToCore(vTaskCore1,        // コールバック関数ポインタ
                            "vTaskCore1",      // タスク名
                            4096,              // スタック
                            NULL,              // パラメータ
                            2,                 // 優先度(0～7、7が最優先)
                            &s_xTaskCore1,     // ハンドル
                            APP_PROC_CORE      // CPUのコア選択
                            );

#ifdef DEBUG_TASK
    // [デバッグ用のタスク @CPU Core 1]
    xTaskCreatePinnedToCore(vTaskDebug,        // コールバック関数ポインタ
                            "vTaskDebug",      // タスク名
                            4096,              // スタック
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