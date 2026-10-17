#pragma once
#include <Arduino.h>

// ==============================================================================
// ESP32-S3-DevKitC-1 N16R8 PIN DEFINITIONS: PROJECT 1
// Title: IoT-Based Rural Lightning Early Warning, 3-Stream TinyML Cloud-Verified Nowcasting
// ==============================================================================

// 1. Cellular SIM Module (SIM800L / A7670C) on UART1
#define PIN_SIM_RX1       17    // Connect to SIM Module TXD
#define PIN_SIM_TX1       18    // Connect to SIM Module RXD
#define PIN_SIM_PWRKEY    21    // Power Key pulse (if available on breakout)
#define PIN_SIM_RST       19    // Hardware Reset

// 2. I2C Bus (Shared for AS3935, BH1750, MLX90614, BMP280, SH1106 OLED)
#define PIN_I2C_SDA       8
#define PIN_I2C_SCL       9

// I2C Device Addresses
#define ADDR_AS3935       0x03  // CJMCU-3935 Franklin Lightning Sensor (SI=VCC)
#define ADDR_BH1750       0x23  // Digital Lux Sensor (ADDR=GND)
#define ADDR_MLX90614     0x5A  // Non-contact Infrared Sky Thermometer
#define ADDR_BMP280       0x76  // Barometric Pressure Sensor (SDO=GND)
#define ADDR_OLED         0x3C  // 1.3-inch SH1106 128x64 OLED

// AS3935 Interrupt Pin
#define PIN_AS3935_IRQ    4     // Active-HIGH Hardware Interrupt from AS3935

// 3. 1-Wire Environmental Bus
#define PIN_DHT22         5     // Digital Temp & Relative Humidity Data Pin

// 4. 3-Zone Hazard Indicator Tower LEDs
#define PIN_LED_GREEN     6     // Safe Zone Indicator (<30% Cloud, No Strike)
#define PIN_LED_YELLOW    7     // Moderate Caution Zone (60% Cloud / Distant Strike)
#define PIN_LED_RED       15    // Danger Zone (<15km Imminent Strike + Storm Cloud)

// 5. Audio Siren & Safety Actuators
#define PIN_SIREN_PWM     16    // PAM8403 Amplifier Audio IN+ (3W Loudspeaker)
#define PIN_GRID_RELAY    14    // 5V Optocoupler Relay (Disconnects Mains AC on Danger)

// 6. MicroSD SPI Bus (Telemetry Flight Blackbox)
#define PIN_SD_CS         10
#define PIN_SD_MOSI       11
#define PIN_SD_SCK        12
#define PIN_SD_MISO       13

// 7. Interactive Winter Lab Simulation Hardware Buttons
#define PIN_BTN_TEST_STORM 1    // Forces SIM API to query active tropical storm GPS
#define PIN_BTN_MUTE_SIREN 2    // Silences loud field siren during live faculty viva
