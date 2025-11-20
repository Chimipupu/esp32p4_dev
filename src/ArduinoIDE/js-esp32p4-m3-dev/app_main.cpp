/**
 * @file app_main.cpp
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief アプリメイン
 * @version 0.1
 * @date 2025-11-20
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */

#include "common.h"
#include "app_main.h"

// ---------------------------------------------------
// [グローバル変数]
const char *p_pcb_name_str = "JS-ESP32P4-M3-DEV";

uint32_t g_cpu_freq_MHz = 0;
uint32_t g_psram_size = 0;

// ---------------------------------------------------
// [Static変数]

// ---------------------------------------------------
// [プロトタイプ宣言]

// ---------------------------------------------------
// [関数]

/**
 * @brief メモリダンプ(16進HEX & Ascii)
 * 
 * @param dump_addr ダンプするメモリの32bitアドレス
 * @param dump_size ダンプするサイズ(Byte)
 */
void show_mem_dump(uint32_t dump_addr, uint32_t dump_size)
{
    Serial.printf("\n[Memory Dump '(addr:0x%04lX)]\n", dump_addr);

    // ヘッダー行を表示
    Serial.printf("Address  ");
    for (int i = 0; i < 16; i++)
    {
        Serial.printf(" %X ", i);
    }

    Serial.printf("| ASCII\n");
    Serial.printf("-------- ");

    for (int i = 0; i < 16; i++)
    {
        Serial.printf("---");
    }
    Serial.printf("| ------\n");

    // 16バイトずつダンプ
    for (uint32_t offset = 0; offset < dump_size; offset += 16)
    {
        Serial.printf("%08lX: ", dump_addr + offset);

        // 16バイト分のデータを表示
        for (int i = 0; i < 16; i++)
        {
            if (offset + i < dump_size) {
                uint8_t data = *((volatile uint8_t*)(dump_addr + offset + i));
                Serial.printf("%02X ", data);
            } else {
                Serial.printf("   ");
            }
        }

        // ASCII表示
        Serial.printf("| ");
        for (int i = 0; i < 16; i++)
        {
            if (offset + i < dump_size) {
                uint8_t data = *((volatile uint8_t*)(dump_addr + offset + i));
                // 表示可能なASCII文字のみ表示
                Serial.printf("%c", (data >= 32 && data <= 126) ? data : '.');
            } else {
                Serial.printf(" ");  // データがない場合は空白を表示
            }
        }
        Serial.printf("\n");
    }
}

/**
 * @brief アプリメイン初期化
 * 
 */
void app_main_init(void)
{
    // [基板名]
    // (例)　PCB: JS-ESP32P4-M3-DEV
    Serial.printf("PCB: %s\n", p_pcb_name_str);

    // [ESPIDFのバージョン取得]
    // (例)　ESP-IDF Version: v5.5.1-710-g8410210c9a
    Serial.printf("ESP-IDF Version: %s\n", esp_get_idf_version());

    // [CPUのクロック周波数取得]
    // (例)　CPU Clock: 360 MHz
    g_cpu_freq_MHz = getCpuFrequencyMhz();
    Serial.printf("CPU Clock: %lu MHz\n", g_cpu_freq_MHz);

    // [PSRAMのサイズ取得]
    // (例)　PSRAM Size: 32 MB
    g_psram_size = ESP.getPsramSize();
    if (g_psram_size > 0) {
        Serial.printf("PSRAM Size: %lu MB\n", g_psram_size / (1024 * 1024));
    } else {
        Serial.printf("PSRAM: Not available\n");
    }

    // [メモリダンプのテスト]
    show_mem_dump((uint32_t)p_pcb_name_str, strlen(p_pcb_name_str));
}

/**
 * @brief アプリメイン
 * 
 */
void app_main(void)
{
    // TODO:
}