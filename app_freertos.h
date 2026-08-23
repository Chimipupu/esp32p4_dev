/**
 * @file app_freertos.h
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief FreeRTOS
 * @version 0.1
 * @date 2026-08-23
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#ifndef APP_FREERTOS_H
#define APP_FREERTOS_H

// #define DEBUG_TASK

void DBG_LOG_PRINT(const char *p_msg, ...);
void app_freertos_init(void);

#endif // APP_FREERTOS_H
