/**
 * @file esp32p4_dev.ino
 * @author Chimipupu(https://github.com/Chimipupu)
 * @brief ArduinoIDE用ファイル
 * @version 0.1
 * @date 2025-11-20
 * 
 * @copyright Copyright (c) 2025 Chimipupu All Rights Reserved.
 * 
 */

#include "app_main.h"

void setup(void)
{
    // H/W初期化
    Serial.begin(115200);
    delay(100);

    // アプリメイン初期化
    app_main_init();
}

void loop(void)
{
    // アプリメイン
    app_main();
}