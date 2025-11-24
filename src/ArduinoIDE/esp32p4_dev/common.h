/**
 * @file common.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief 共通ヘッダー
 * @version 0.1
 * @date 2025-11-20
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */

#ifndef COMMON_H
#define COMMON_H
// ---------------------------------------------------
// [Include]

// C Std Lib
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// ESP-IDF
#include <esp_system.h>

// ArduinoIDE
#include <Arduino.h>

// FreeRTOS
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

// My Src Inc

// ---------------------------------------------------
// [Define]
#define DRV_CPU_CORE     0 // ドライバ専用CPU @Core 0
#define APP_PROC_CORE    1 // アプリ専用CPU @Core 1
// ---------------------------------------------------
// [コンパイルスイッチ]

// ---------------------------------------------------
// [Global]

// ---------------------------------------------------
// [関数マクロ]

// ---------------------------------------------------
// [プロトタイプ宣言]

#endif // COMMON_H