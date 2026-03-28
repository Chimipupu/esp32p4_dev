/**
 * @file common.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief 共通ヘッダー
 * @version 0.1
 * @date 2026-03-28
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
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
// [コンパイルスイッチ]

// ---------------------------------------------------
// [Define]
#define DRV_CPU_CORE     0 // ドライバ専用CPU @Core 0
#define APP_PROC_CORE    1 // アプリ専用CPU   @Core 1

// ---------------------------------------------------
// レジスタを8/16/32bitでR/Wするマクロ
#define REG_READ_BYTE(base, offset)           (*(volatile uint8_t  *)((base) + (offset)))
#define REG_READ_WORD(base, offset)           (*(volatile uint16_t *)((base) + (offset)))
#define REG_READ_DWORD(base, offset)          (*(volatile uint32_t *)((base) + (offset)))
#define REG_WRITE_BYTE(base, offset, val)     (*(volatile uint8_t  *)((base) + (offset)) = (val))
#define REG_WRITE_WORD(base, offset, val)     (*(volatile uint16_t *)((base) + (offset)) = (val))
#define REG_WRITE_DWORD(base, offset, val)    (*(volatile uint32_t *)((base) + (offset)) = (val))

// レジスタビット操作
#define REG_BIT_SET(reg, bit)                 ((reg) |=  (1UL << (bit))) // レジスタのビットをセット
#define REG_BIT_CLR(reg, bit)                 ((reg) &= ~(1UL << (bit))) // レジスタのビットをクリア
#define REG_BIT_TGL(reg, bit)                 ((reg) ^=  (1UL << (bit))) // レジスタのビットをトグル
#define REG_BIT_CHK(reg, bit)                 ((reg) &   (1UL << (bit))) // レジスタのビットチェック

// ---------------------------------------------------
// NOP
static inline void ASM_NOP(void)
{
    __asm__ __volatile__("nop");
}


#endif // COMMON_H