/**
 * @file pcb_def.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief 基板別定義
 * @version 0.1
 * @date 2026-03-28
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 */

#ifndef PCB_DEF_H
#define PCB_DEF_H

// ---------------------------------------------------
/**
 * @brief WT9932P4_TINY
 * @note Flash: 16MB, PSRAM: 32MB
 * @note WiFi/Bluetooth: (N/A)
 */
#define WT9932P4_TINY        0x00

/**
 * @brief JS_ESP32P4_M3_DEV
 * @note Flash: 16MB, PSRAM: 32MB
 * @note WiFi/Bluetooth: ESP32-C6
 */
#define JS_ESP32P4_M3_DEV    0x01

// #define PCB_TYPE             WT9932P4_TINY
#define PCB_TYPE             JS_ESP32P4_M3_DEV
// ---------------------------------------------------

#endif // PCB_DEF_H
