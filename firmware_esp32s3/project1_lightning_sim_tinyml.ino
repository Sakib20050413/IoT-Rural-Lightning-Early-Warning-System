// =============================================================================
// PROJECT 1 (CSE 301): RURAL AI LIGHTNING, SKY CLOUD & SIM WEB-GIS NODE
// Target MCU: ESP32-S3-DevKitC-1 N16R8 Dual Type-C (44-Pin)
// Team BlackBerry (ID: 1119003, 1119004, 1119007) — BAIUST, Cumilla
// =============================================================================
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

// Pin Definitions for ESP32-S3 N16R8 (44-Pin)
#define I2C_SDA_PIN       8
#define I2C_SCL_PIN       9
#define AS3935_IRQ_PIN    4
#define DHT22_PIN         5
#define LED_GREEN_PIN     6
#define LED_YELLOW_PIN    7
#define LED_RED_PIN       15
#define RELAY_GRID_PIN    14
#define BUZZER_PIN        16
#define SIM800_RX_PIN     17
#define SIM800_TX_PIN     18
#define DFPLAYER_RX_PIN   41
#define DFPLAYER_TX_PIN   42
#define SD_CS_PIN         10
#define RF_EMULATOR_PIN   19

// 3-Stream Sensor State
float skyLux = 620.0;
float skyIrTempC = -11.5;
float ambientTempC = 26.5;
float humidityPct = 55.0;
float pressureHpa = 1013.0;
float simApiCloudPct = 15.0;
int strikeDistKm = 99;
int tinymlRiskScore = 8;

void setHazardZone(const char* zone) {
  digitalWrite(LED_GREEN_PIN,  strcmp(zone, "GREEN") == 0 ? HIGH : LOW);
  digitalWrite(LED_YELLOW_PIN, strcmp(zone, "YELLOW") == 0 ? HIGH : LOW);
  digitalWrite(LED_RED_PIN,    strcmp(zone, "RED") == 0 ? HIGH : LOW);
  // Active-low or active-high relay cutoff in RED zone
  digitalWrite(RELAY_GRID_PIN, strcmp(zone, "RED") == 0 ? LOW : HIGH);
}

void runTinyML3StreamFusion() {
  // 1. Direct Sky Cloud Verification (Rejects high humidity if sky is bright & cold)
  float deltaTemp = ambientTempC - skyIrTempC;
  bool physicalCloudDetected = (skyLux < 60.0) && (deltaTemp < 10.0);
  bool pressureDropping = (pressureHpa < 1002.0);
  bool simCloudVerified = (simApiCloudPct > 65.0);

  // 2. Multi-Stream TinyML Risk Fusion
  if (strikeDistKm <= 15 && (physicalCloudDetected || simCloudVerified || pressureDropping)) {
    tinymlRiskScore = 96;
    setHazardZone("RED");
  } else if ((strikeDistKm > 15 && strikeDistKm <= 30) || (physicalCloudDetected && simCloudVerified)) {
    tinymlRiskScore = 68;
    setHazardZone("YELLOW");
  } else {
    tinymlRiskScore = 10;
    setHazardZone("GREEN");
  }

  // Output JSON for WebSerial (2nd Type-C port) & UART0
  Serial.printf("{"lux":%.1f,"skyTemp":%.1f,"pres":%.1f,"hum":%.1f,"apiCloud":%.1f,"distKm":%d,"risk":%d}
",
                skyLux, skyIrTempC, pressureHpa, humidityPct, simApiCloudPct, strikeDistKm, tinymlRiskScore);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(RELAY_GRID_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  setHazardZone("GREEN");
  Serial.println("BAIUST CSE-301 ESP32-S3 Lightning & SIM-GIS Node Ready.");
}

void loop() {
  runTinyML3StreamFusion();
  delay(1000);
}
