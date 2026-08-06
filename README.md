# ESP32-P4 評価F/W個人開発

## 開発環境

###  H/W

  - SoC
    - ESP32-P4
      - FPU: 単精度
      - Flash: 16MB
      - CPU
        - 32bit RISC-V `RV32IMAFC`
        - コア数: x2
      - Clock
        - 360MHz
          - ※チップリビジョンがv1.0~v3.0のみ
        - 400MHz
          - ※チップリビジョンがv3.1以降のみ
      - FPU
        - 単精度
      - RAM
        - SRAM: 768
        - PSRAM: 32MB
    - デバッガ
        - ESP32-P4内蔵 JTAG
    - 基板
        - `WT9932P4-TINY`
            - RGBLED: WS2812 x1個
            - WiFi/Bluetooth: (N/A)
        - `JS-ESP32P4-M3-DEV`
            - RGBLED: (N/A)
            - WiFi/Bluetooth: ESP32-C6

### S/W

  - ドライバ
    - ESP-IDF v5.5.5
  - OS
    - FreeRTOS
  - IDE
    - Arduino IDE v2.3.10

<div align="left">
  <img src="/doc/Arduino_build_config_js-esp32p4-m3-dev.png">
</div>
