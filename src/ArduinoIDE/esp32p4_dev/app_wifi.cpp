/**
 * @file app_wifi.cpp
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief WiFiアプリ (for ESP32-P4)
 * @note ESP32-P4とSDIOで接続されてるESP32-C6専用の実装
 * @version 0.1
 * @date 2026-08-06
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#include "app_wifi.h"
#include "time.h"

// -----------------------------------------------------------
typedef struct {
    wifi_auth_mode_t auth_mode;
    const char *p_auth_str;
} app_wifi_auth_data_t;

static const char *gp_ntp_server_tbl[] = {
    "ntp.nict.jp",
    "time.google.com",
    "ntp.jst.mfeed.ad.jp",
};

static const app_wifi_auth_data_t g_wifi_auth_data_tbl[] = {
    {WIFI_AUTH_OPEN,            "open"},
    {WIFI_AUTH_WEP,             "WEP"},
    {WIFI_AUTH_WPA_PSK,         "WPA"},
    {WIFI_AUTH_WPA2_PSK,        "WPA2"},
    {WIFI_AUTH_WPA_WPA2_PSK,    "WPA+WPA2"},
    {WIFI_AUTH_WPA2_ENTERPRISE, "WPA2-EAP"},
    {WIFI_AUTH_WPA3_PSK,        "WPA3"},
    {WIFI_AUTH_WPA2_WPA3_PSK,   "WPA2+WPA3"},
    {WIFI_AUTH_WAPI_PSK,        "WAPI"},
};
static const uint8_t WIFI_AUTH_DATA_TBL_SIZE = sizeof(g_wifi_auth_data_tbl) / sizeof(g_wifi_auth_data_tbl[0]);

#define WIFI_ERROR_NONE         0
#define WIFI_ERROR_UNKNOWN      0xFF
static uint8_t s_wifi_err = WIFI_ERROR_NONE;
static time_t s_utc_time = 0;
// static time_t s_jst_time = 0;
static struct tm *sp_utc_tm = NULL;
static struct tm *sp_jst_tm = NULL;
static bool s_is_ntp_sync = false;
static bool s_is_wifi_connect = false;

static void _wifi_event_handler(WiFiEvent_t event, WiFiEventInfo_t info);
static void _wifi_coprocessor_reset(void);
static void _wifi_connet(const char *p_ssid, const char *p_password);
// static void _wifi_disconnet(void);
static bool _get_wifi_auth_mode_data(wifi_auth_mode_t auth_mode);
static void _get_ntp_and_rtc_time(void);
// -----------------------------------------------------------
// [Static]

static void _wifi_event_handler(WiFiEvent_t event, WiFiEventInfo_t info)
{
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_START:
            Serial.printf("\r\n[WiFi Event] WiFi STA: Started\r\n");
            break;

        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.printf("\r\n[WiFi Event] WiFi STA: Connected to AP\r\n");
            s_is_wifi_connect = true;
            break;

        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.printf("\r\n[WiFi Event] WiFi STA: Got IP Address\r\n");
            break;

        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            s_is_wifi_connect = false;
            s_wifi_err = info.wifi_sta_disconnected.reason;
            // Serial.printf("\r\n[WiFi Event] WiFi STA: Disconnected. Reason = %d\r\n", s_wifi_err);
            break;

        // Caseで取ってない知らんイベント
        default:
            Serial.printf("\r\n[WiFi Event] WiFi STA: My App, Not Support Event\r\n");
            break;
    }
}

/**
 * @brief WiFiコプロセッサ初期化(for ESP32P4 <-> ESP32-C6)
 */
static void _wifi_coprocessor_reset(void)
{
    Serial.printf("WiFi Co-Processor Init\r\n");

    WiFi.mode(WIFI_STA);
    // WiFi.disconnect();
    delay(100);
}

static void _wifi_connet(const char *p_ssid, const char *p_password)
{
    uint8_t retry_count;

    // WiFiイベントハンドラを登録
    WiFi.onEvent(_wifi_event_handler);

    if((p_ssid == NULL) && (p_password == NULL)) {
        Serial.printf("WiFi STA Mode\r\n");

        _wifi_coprocessor_reset();

        WiFi.STA.begin();
        return;
    }

    Serial.printf("SSID: %s\r\n", p_ssid);
    Serial.printf("Password: %s\r\n", p_password);
    Serial.printf("WiFi Connectting\r\n");

    // WiFiコプロセッサのリセット
    _wifi_coprocessor_reset();

WIFI_RETRY:
    WiFi.begin(p_ssid, p_password);
    retry_count = 0;

#if 1
    while((WiFi.status() != WL_CONNECTED) || (s_is_wifi_connect == false))
    {
        delay(1000);
        Serial.print(".");
        retry_count++;

        // WiFiイベントハンドラで何かしらの切断イベントが発生
        if(s_wifi_err != WIFI_ERROR_NONE) {
            // エラーコードだけ表示して、フラグをリセット
            Serial.printf("\r\n[WARN] WiFi disconnected (Reason: %d), Retrying...\r\n", s_wifi_err);

            // NOTE: WIFI_REASON_ASSOC_LEAVE (10進で8) の「Deassociated due to leaving」は無視して大丈夫
            if(s_wifi_err == WIFI_REASON_ASSOC_LEAVE) {
                s_wifi_err = WIFI_ERROR_NONE;
                goto WIFI_RETRY;
            }
            // WiFiのエラー発生
            else {
                s_wifi_err = WIFI_ERROR_UNKNOWN;
                return;
            }
        }

        // タイムアウト
        if (retry_count > 60) {
            Serial.printf("\r\n[ERROR] WiFi Connection Timeout!\r\n");
            return;
        }
    }
#endif

    Serial.printf("\r\nWiFi Connect, Succes!\r\n");
    Serial.printf("RSSI: %d\r\n", WiFi.RSSI());
    Serial.printf("IP Addr: %s\r\n", WiFi.localIP().toString().c_str());
}

#if 0
static void _wifi_disconnet(void)
{
    s_is_wifi_connect = false;
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.printf("WiFi Disconnected!\r\n");
}
#endif

static bool _get_wifi_auth_mode_data(wifi_auth_mode_t auth_mode, app_wifi_auth_data_t *p_output_data)
{
    bool ret;
    uint8_t i;

    if(p_output_data == NULL) {
        return false;
    }

    ret = false;
    p_output_data->p_auth_str = "none";

    for(i = 0; i < WIFI_AUTH_DATA_TBL_SIZE; i++)
    {
        if(g_wifi_auth_data_tbl[i].auth_mode == auth_mode) {
            *p_output_data = g_wifi_auth_data_tbl[i];
            ret = true;
        }
    }

    return ret;
}

static void _get_ntp_and_rtc_time(void)
{
    struct tm utc_tm;
    struct tm jst_tm;

    if(s_is_ntp_sync != true) {
        // NTPに接続してUTCとJSTを取得
        configTime(NTP_TIMEZONE_JST, 0, gp_ntp_server_tbl[0], gp_ntp_server_tbl[1], gp_ntp_server_tbl[2]);
        s_utc_time = time(nullptr);
        sp_utc_tm = gmtime_r(&s_utc_time, &utc_tm);

        // NTPと時刻同期が未完了なので終了
        if(sp_utc_tm->tm_year <= 70) {
            return;
        }

        // UTCからJSTに変換
        sp_jst_tm = localtime_r(&s_utc_time, &jst_tm);

        // NTPと時刻同期したのでフラグを降ろす
        s_is_ntp_sync = true;
        Serial.printf("NTP time Sync RTC Succes!\r\n");

        // WiFi切断
        // _wifi_disconnet();
    } else {
        // ESP32内蔵のRTCから時刻を取得
        s_utc_time = time(nullptr);
        sp_utc_tm = gmtime_r(&s_utc_time, &utc_tm);
        sp_jst_tm = localtime_r(&s_utc_time, &jst_tm);
        Serial.printf("Get RTC Time\r\n");
    }

    // 時刻表示
#if 0
    Serial.printf("UTC Time: %04d/%02d/%02d %02d:%02d:%02d\r\n",
        sp_utc_tm->tm_year + 1900, sp_utc_tm->tm_mon + 1, sp_utc_tm->tm_mday,
        sp_utc_tm->tm_hour, sp_utc_tm->tm_min, sp_utc_tm->tm_sec);
#else
    Serial.printf("JST Time: %04d/%02d/%02d %02d:%02d:%02d\r\n",
        sp_jst_tm->tm_year + 1900, sp_jst_tm->tm_mon + 1, sp_jst_tm->tm_mday,
        sp_jst_tm->tm_hour, sp_jst_tm->tm_min, sp_jst_tm->tm_sec);
#endif
}
// -----------------------------------------------------------
// [API]

void app_wifi_init(const char *p_ssid, const char *p_password)
{
    _wifi_connet(p_ssid, p_password);
}

void app_wifi_main(void)
{
    // WiFiエラー発生中は処理しない
    if(s_wifi_err == WIFI_ERROR_UNKNOWN) {
        return;
    }

    _get_ntp_and_rtc_time(); // NTPとの時刻同期
}