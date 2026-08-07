/**
 * @file esp32p4_dev.ino
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief ArduinoIDE用ファイル
 * @version 0.1
 * @date 2026-08-06
 * @copyright Copyright (c) 2026 Chimipupu All Rights Reserved.
 */

#include "app_main.h"

void setup(void)
{
    app_main_init();
}

void loop(void)
{
    app_main(); // アプリメイン
}