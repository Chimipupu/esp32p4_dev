/**
 * @file app_main.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief アプリメイン
 * @version 0.1
 * @date 2026-08-23
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#ifndef APP_MAIN_H
#define APP_MAIN_H

#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include "common.h"
#include "pcb_def.h"

// ---------------------------------------------------
// [コンパイルスイッチ]
// #define DEBUG_APP

// ---------------------------------------------------
void app_main_init(void);
void app_main(void);

#endif // APP_MAIN_H
