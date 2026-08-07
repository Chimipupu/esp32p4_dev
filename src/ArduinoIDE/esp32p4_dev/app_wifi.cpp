/**
 * @file app_wifi.cpp
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief WiFiアプリ
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

#define WIFI_ERROR_NONE    0
static wifi_err_reason_t s_wifi_err = (wifi_err_reason_t) WIFI_ERROR_NONE;
static time_t s_utc_time = 0;
// static time_t s_jst_time = 0;
static struct tm *sp_utc_tm = NULL;
static struct tm *sp_jst_tm = NULL;
static bool s_is_ntp_sync = false;

static void _wifi_event_handler(WiFiEvent_t event, WiFiEventInfo_t info);
static void _wifi_coprocessor_reset(void);
static void _wifi_connet(const char *p_ssid, const char *p_password);
static void _wifi_disconnet(void);
static bool _get_wifi_auth_mode_data(wifi_auth_mode_t auth_mode);
static void _get_ntp_and_rtc_time(void);
// -----------------------------------------------------------
// [Static]

static void _wifi_event_handler(WiFiEvent_t event, WiFiEventInfo_t info)
{
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_START:
            Serial.printf("[WiFi Event] Station Started\r\n");
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.printf("[WiFi Event] Connected to AP\r\n");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.printf("[WiFi Event] Got IP Address\r\n");
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            s_wifi_err = (wifi_err_reason_t) info.wifi_sta_disconnected.reason;
            Serial.printf("[WiFi Event] Disconnected! Reason: %d\r\n", s_wifi_err);
            break;
        default:
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

    _wifi_coprocessor_reset();
    delay(500);  // WiFiコプロセッサのリセット待ち

WIFI_RETRY:
    WiFi.begin(p_ssid, p_password);
    retry_count = 0;

#if 1
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
        retry_count++;

        // WiFiイベントハンドラで何かしらの切断イベントが発生
        if(s_wifi_err != WIFI_ERROR_NONE) {
            // エラーコードだけ表示して、フラグをリセット
            // NOTE: wifi_err_reason_tの8は「Deassociated due to leaving」で初期化時の仕様で出ることが多いから無視して大丈夫
            Serial.printf("\r\n[WARN] WiFi disconnected (Reason: %d), Retrying...\r\n", s_wifi_err);
            s_wifi_err = (wifi_err_reason_t) WIFI_ERROR_NONE;
            goto WIFI_RETRY;
        }

        // タイムアウト処理
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

static void _wifi_disconnet(void)
{
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.printf("WiFi Disconnected!\r\n");
}

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
#if 0
void app_wifi_scan(void)
{
    uint8_t i;
    wifi_auth_mode_t wifi_auth_mode;
    uint8_t scan_cnt;
    app_wifi_auth_data_t wifi_auth_data;

    Serial.println("-------------------------------------");
    Serial.println("[WiFi Scan]");
    WiFi.setBandMode(WIFI_BAND_MODE_AUTO);

    scan_cnt = WiFi.scanNetworks();

    if (scan_cnt == 0) {
        Serial.println("WiFi not found");
    } else {
        Serial.printf("WiFi Found (%d)\r\n", scan_cnt);
        Serial.println("No | SSID                             | RSSI | CH | Encryption");

        for (i = 0; i < scan_cnt; ++i)
        {
            Serial.printf("%2d", i + 1);
            Serial.print(" | ");
            Serial.printf("%-32.32s", WiFi.SSID(i).c_str());
            Serial.print(" | ");
            Serial.printf("%4" PRIi32, WiFi.RSSI(i));
            Serial.print(" | ");
            Serial.printf("%2" PRIi32, WiFi.channel(i));
            Serial.print(" | ");

            wifi_auth_mode = WiFi.encryptionType(i);
            _get_wifi_auth_mode_data(wifi_auth_mode, &wifi_auth_data);
            Serial.printf("%s\r\n", wifi_auth_data.p_auth_str);

            delay(10);
        }
    }

    WiFi.scanDelete();
    Serial.println("-------------------------------------");
}
#endif

void app_wifi_init(const char *p_ssid, const char *p_password)
{
    _wifi_connet(p_ssid, p_password);
}

void app_wifi_main(void)
{
    _get_ntp_and_rtc_time();
}