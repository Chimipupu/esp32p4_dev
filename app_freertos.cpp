/**
 * @file app_freertos.cpp
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief FreeRTOS
 * @version 0.1
 * @date 2026-08-23
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#include "app_freertos.h"
#include "app_main.h"
#include "common.h"
#include "pcb_def.h"
#include "dbg_cmd.h"

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
#define QUE_SIZE_DEBUG_LOG    32

typedef struct {
    char *p_msg;
} log_msg_t;

static QueueHandle_t s_queue_handle_log = NULL;

static xTaskHandle s_xTaskDebugLog;
static xTaskHandle s_xTaskDebugCmd;

static E_DBG_CMD_RESULT _cmd_test(void *p_args);
static const dbg_cmd_tbl_t s_ext_cmd_tbl[] = {
    {"test", "ts", _cmd_test},
};

static uint8_t _serial_read_func(void);

static const dbg_cmd_config_t s_dbg_cmd_config = {
    .p_serial_read = _serial_read_func,
    .p_printf = DBG_LOG_PRINT,
    .p_ext_cmd_tbl = (dbg_cmd_tbl_t *)s_ext_cmd_tbl,
    .ext_cmd_num = sizeof(s_ext_cmd_tbl) / sizeof(s_ext_cmd_tbl[0]),
};

static void vTaskDebugLog(void *p_param);
static void vTaskDebugCmd(void *p_param);

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
// [Static関数]

static uint8_t _serial_read_func(void)
{
    return (uint8_t)Serial.read();
}

static E_DBG_CMD_RESULT _cmd_test(void *p_args)
{
    DBG_LOG_PRINT("\33[31mTest Cmd Exec\r\n\33[0m");
    return CMD_RESULT_EXEC_OK;
}

#ifdef DEBUG_TASK
static void _print_task_status(void)
{
    char *p_buf;

    p_buf = (char *)ps_malloc(1024);

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

    DBG_LOG_PRINT("[CPU Core %d] vTaskCore0\n", s_core_num);

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
            DBG_LOG_PRINT("[vTaskRgbLed] RGB_LED: Set Color = (0x%02X, 0x%02X, 0x%02X)\n",
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

    DBG_LOG_PRINT("[CPU Core %d] vTaskCore1\n", s_core_num);

    while (1)
    {
#ifdef RGBLED_USE
        que_status = xQueueSend(s_queue_handle_rgbled_data,
                                &s_request_rgb_led_val,
                                portMAX_DELAY);

        // キューの送信完了を受けて次のデータを用意しておく
        if(que_status == pdPASS)
        {
#if 0
            DBG_LOG_PRINT("[vTaskCore1] RGB_LED: Request Color = (0x%02X, 0x%02X, 0x%02X)\n",
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
            Serial.printf("%s", log_msg.p_msg);
            free(log_msg.p_msg);
        }
    }
}

static void vTaskDebugCmd(void *p_param)
{
    dbg_cmd_init((dbg_cmd_config_t *) &s_dbg_cmd_config);

    while (1)
    {
        dbg_cmd_poll();
        dbg_cmd_main();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
// ---------------------------------------------------
// [APP]

void DBG_LOG_PRINT(const char *p_msg, ...)
{
    log_msg_t log_msg;
    va_list args;
    int msg_len;

    if(p_msg != NULL)
    {
        va_start(args, p_msg);
        msg_len = vsnprintf(NULL, 0, p_msg, args);
        va_end(args);

        if(msg_len >= 0)
        {
            log_msg.p_msg = (char *)ps_malloc(msg_len + 1);

            if(log_msg.p_msg != NULL)
            {
                va_start(args, p_msg);
                vsnprintf(log_msg.p_msg, msg_len + 1, p_msg, args);
                va_end(args);

                xQueueSend(s_queue_handle_log, &log_msg, 0);
            }
        }
    }
}

void app_freertos_init(void)
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

    // [デバッグ用のUARTコマンドを処理するタスク @CPU Core 1]
    xTaskCreatePinnedToCore(vTaskDebugCmd,     // コールバック関数ポインタ
                            "vTaskDebugCmd",   // タスク名
                            4096,              // スタック
                            NULL,              // パラメータ
                            1,                 // 優先度(0～7、7が最優先)
                            &s_xTaskDebugCmd,  // ハンドル
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