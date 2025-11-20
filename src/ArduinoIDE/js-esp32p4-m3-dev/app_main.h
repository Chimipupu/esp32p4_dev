/**
 * @file app_main.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief アプリメインのヘッダ
 * @version 0.1
 * @date 2025-11-20
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */

#ifndef APP_MAIN_H
#define APP_MAIN_H

#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

// レジスタを8/16/32bitでR/Wするマクロ
#define REG_READ_BYTE(base, offset)         (*(volatile uint8_t  *)((base) + (offset)))
#define REG_READ_WORD(base, offset)         (*(volatile uint16_t *)((base) + (offset)))
#define REG_READ_DWORD(base, offset)        (*(volatile uint32_t *)((base) + (offset)))
#define REG_WRITE_BYTE(base, offset, val)   (*(volatile uint8_t  *)((base) + (offset)) = (val))
#define REG_WRITE_WORD(base, offset, val)   (*(volatile uint16_t *)((base) + (offset)) = (val))
#define REG_WRITE_DWORD(base, offset, val)  (*(volatile uint32_t *)((base) + (offset)) = (val))

// レジスタビット操作
#define REG_BIT_SET(reg, bit)               ((reg) |=  (1UL << (bit))) // レジスタのビットをセット
#define REG_BIT_CLR(reg, bit)               ((reg) &= ~(1UL << (bit))) // レジスタのビットをクリア
#define REG_BIT_TGL(reg, bit)               ((reg) ^=  (1UL << (bit))) // レジスタのビットをトグル
#define REG_BIT_CHK(reg, bit)               ((reg) &   (1UL << (bit))) // レジスタのビットチェック

// NOP
static inline void ASM_NOP(void)
{
    __asm__ __volatile__("nop");
}

void show_mem_dump(uint32_t dump_addr, uint32_t dump_size);
void app_main_init(void);
void app_main(void);

#endif // APP_MAIN_H
