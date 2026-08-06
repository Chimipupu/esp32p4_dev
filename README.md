# ESP32-P4 評価F/W個人開発

## 開発環境

### S/W

  - IDE
    - Arduino IDE v2.3.10
  - ドライバ
    - ESPIDF v5.5.5
  - OS
    - FreeRTOS

<div align="left">
  <img src="/doc/Arduino_build_config_js-esp32p4-m3-dev.png">
</div>

###  H/W

  - SoC
    - ESP32-P4
      - FPU: 単精度
      - Flash: 16MB
      - RAM
        - SRAM: 
        - PSRAM: 32MB
      - Clock: 360MHz
    - デバッガ
        - ESP32-P4内蔵 JTAG
    - 基板
        - WT9932P4-TINY
        - JS-ESP32P4-M3-DEV
            - ESP32 Chip Model: ESP32-P4
            - ESP32 Chip Rev: v1.0
            - CPU Clock: 360 MHz
            - CPU Core: x2
            - Flash Size: 16 MB
            - PSRAM Size: 32 MB
