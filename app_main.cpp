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
#define QUE_SIZE_RGBLED_DATA    8
static QueueHandle_t s_queue_handle_rgbled_data = NULL;
static led_color_t s_request_rgb_led_val = {.rgb = 0x000000};
static led_color_t s_local_rgb_led_val = {.rgb = 0x000000};
static void vTaskRgbLed(void *p_param);
#endif

#ifdef WIFI_USE
#include "app_wifi.h"
static const char *g_wifi_ssid = MY_WIFI_SSID;
static const char *g_wifi_password = MY_WIFI_PASSWORD;
#endif

// ---------------------------------------------------
// [DEBUG関連]
#define QUE_SIZE_DEBUG_LOG    8
static xTaskHandle s_xTaskDebugLog;
static QueueHandle_t s_queue_handle_log = NULL;

static void _dbg_pcb_info_print(void);
static void DBG_LOG_PRINT(QueueHandle_t queue_handle, const char *p_msg, ...);
static void vTaskDebugLog(void *p_param);

#ifdef DEBUG_TASK
static void _print_task_status(void);
#endif // DEBUG_TASK
// ---------------------------------------------------
// [FreeRTOS関連]
#define QUE_SIZE_CORE_COM    8

typedef struct {
    bool is_test_mode;
    uint8_t test_mode_val;
} core_com_data_t;

static xTaskHandle s_xTaskCore0;
static xTaskHandle s_xTaskCore1;
static xTaskHandle s_xTaskRgbLed;
static QueueHandle_t s_queue_handle_core_com = NULL;

static void vTaskCore0(void *p_param);
static void vTaskCore1(void *p_param);

// ---------------------------------------------------
typedef struct {
    char msg[128];
} log_msg_t;

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
static void _freertos_init(void);
static void _mcu_init(void);
static void _pcb_init(void);

// ---------------------------------------------------
// [Static関数]

static void DBG_LOG_PRINT(QueueHandle_t queue_handle, const char *p_msg, ...)
{
    log_msg_t log_msg;
    va_list args;
    UBaseType_t que_space;

    va_start(args, p_msg);
    vsnprintf(log_msg.msg, sizeof(log_msg.msg), p_msg, args);
    va_end(args);

    que_space = uxQueueSpacesAvailable(queue_handle);
    if(que_space > 0)
    {
        xQueueSend(queue_handle, &log_msg, 0);
    } else {
        Serial.printf("\33[31m [ERROR] Log Queue Full: %u\n\33[0m", que_space);
    }
}

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

#ifdef DEBUG_TASK
static void _print_task_status(void)
{
    char *p_buf;

    p_buf = (char *)malloc(1024);

    if (p_buf != NULL)
    {
        Serial.printf("--- Task List ---\n");
        vTaskList(p_buf);
        Serial.printf("%s\n", p_buf);

        Serial.printf("--- Run Time Stats ---\n");
        vTaskGetRunTimeStats(p_buf);
        Serial.printf("%s\n", p_buf);

        free(p_buf);
    }
}
#endif // DEBUG_TASK
// ---------------------------------------------------
// [FreeRTOSタスク(CPU Core0)]

static void vTaskCore0(void *p_param)
{
    BaseType_t que_status;
    static uint8_t s_core_num = xPortGetCoreID();

    DBG_LOG_PRINT(s_queue_handle_log, "[CPU Core %d] vTaskCore0\n", s_core_num);

#ifdef RGBLED_USE
    xTaskCreatePinnedToCore(vTaskRgbLed,       // コールバック関数ポインタ
                            "vTaskRgbLed",     // タスク名
                            1024,              // スタック
                            NULL,              // パラメータ
                            1,                 // 優先度(0～7、7が最優先)
                            &s_xTaskRgbLed,    // ハンドル
                            DRV_CPU_CORE       // CPUのコア選択
                            );
#endif

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

    while (1)
    {
        que_status = xQueueReceive(s_queue_handle_core_com,
                                    &s_local_rgb_led_val,
                                    portMAX_DELAY);

        if(que_status == pdPASS)
        {
            // TODO
        }
    }
}

#ifdef RGBLED_USE
static void vTaskRgbLed(void *p_param)
{
    BaseType_t que_status;

    while (1)
    {
        que_status = xQueueReceive(s_queue_handle_rgbled_data,
                                    &s_local_rgb_led_val,
                                    portMAX_DELAY);

        if(que_status == pdPASS)
        {
            app_neopixel_set_rgb(0, &s_local_rgb_led_val);
#if 0
            DBG_LOG_PRINT(s_queue_handle_log,
                    "[vTaskRgbLed] RGB_LED: Set Color = (0x%02X, 0x%02X, 0x%02X)\n",
                    s_local_rgb_led_val.para.red,
                    s_local_rgb_led_val.para.green,
                    s_local_rgb_led_val.para.blue);
#endif
        }
    }
}
#endif

// ---------------------------------------------------
// [FreeRTOSタスク(CPU Core1)]

static void vTaskCore1(void *p_param)
{
    BaseType_t que_status;

    static uint8_t s_tbl_idx = 0;
    static uint8_t s_core_num = xPortGetCoreID();

    DBG_LOG_PRINT(s_queue_handle_log, "[CPU Core %d] vTaskCore1\n", s_core_num);

    while (1)
    {
#ifdef RGBLED_USE
        que_status = xQueueSend(s_queue_handle_rgbled_data,
                                &s_request_rgb_led_val,
                                portMAX_DELAY);

        // キューの送信完了を受けて次のデータを用意しておく
        if(que_status == pdPASS)
        {
#if 1
            DBG_LOG_PRINT(s_queue_handle_log,
                    "[vTaskCore1] RGB_LED: Request Color = (0x%02X, 0x%02X, 0x%02X)\n",
                    s_request_rgb_led_val.para.red,
                    s_request_rgb_led_val.para.green,
                    s_request_rgb_led_val.para.blue);
#endif
            s_request_rgb_led_val.rgb = g_led_color_tbl[s_tbl_idx].rgb.rgb;
            s_tbl_idx = (s_tbl_idx + 1) % (RGBLED_COLOR_TBL_SIZE - 1);
        }
#endif

#ifdef DEBUG_TASK
        _print_task_status();
#endif // DEBUG_TASK

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void vTaskDebugLog(void *p_param)
{
    BaseType_t que_status;
    log_msg_t log_msg;

    while (1)
    {
        que_status = xQueueReceive(s_queue_handle_log,
                                    &log_msg,
                                    portMAX_DELAY);

        if(que_status == pdPASS)
        {
            Serial.printf("%s", log_msg.msg);
            memset(&log_msg.msg[0], 0x00, sizeof(log_msg.msg));
        }
    }
}

static void _freertos_init(void)
{
    // キューの作成
    s_queue_handle_core_com = xQueueCreate(QUE_SIZE_CORE_COM, sizeof(core_com_data_t));
    s_queue_handle_log = xQueueCreate(QUE_SIZE_DEBUG_LOG, sizeof(log_msg_t));
#ifdef RGBLED_USE
    s_queue_handle_rgbled_data = xQueueCreate(QUE_SIZE_RGBLED_DATA, sizeof(led_color_t));
#endif

    // [デバッグ用のログをprintf()するだけのタスク @CPU Core 1]
    xTaskCreatePinnedToCore(vTaskDebugLog,     // コールバック関数ポインタ
                            "vTaskDebugLog",   // タスク名
                            4096,              // スタック
                            NULL,              // パラメータ
                            0,                 // 優先度(0～7、7が最優先)
                            &s_xTaskDebugLog,  // ハンドル
                            APP_PROC_CORE      // CPUのコア選択
                            );

    // [RTOSタスク @CPU Core 0]
    xTaskCreatePinnedToCore(vTaskCore0,        // コールバック関数ポインタ
                            "vTaskCore0",      // タスク名
                            4096,              // スタック
                            NULL,              // パラメータ
                            3,                 // 優先度(0～7、7が最優先)
                            &s_xTaskCore0,     // ハンドル
                            DRV_CPU_CORE       // CPUのコア選択
                            );

    // [RTOSタスク @CPU Core 1]
    xTaskCreatePinnedToCore(vTaskCore1,        // コールバック関数ポインタ
                            "vTaskCore1",      // タスク名
                            4096,              // スタック
                            NULL,              // パラメータ
                            2,                 // 優先度(0～7、7が最優先)
                            &s_xTaskCore1,     // ハンドル
                            APP_PROC_CORE      // CPUのコア選択
                            );
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