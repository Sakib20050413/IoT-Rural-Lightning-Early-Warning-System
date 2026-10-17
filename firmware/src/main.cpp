#include <Arduino.h>
#include <Wire.h>
#include "pinout.h"
#include "sim_manager.h"
#include "sensor_fusion.h"

// System instances
SIMManager sim;
SensorFusionEngine fusion;
SensorReadings currentReadings;

volatile bool as3935InterruptFired = false;
volatile unsigned long lastInterruptTime = 0;

void IRAM_ATTR onAS3935Interrupt() {
    as3935InterruptFired = true;
    lastInterruptTime = millis();
}

// Siren sound generation using ESP32 PWM
void playFieldSiren(bool enable) {
    if (!enable) {
        ledcWrite(0, 0); // Duty 0
        return;
    }
    // High-pitched dual-tone field evacuation wail
    for (int freq = 700; freq <= 1500; freq += 50) {
        ledcWriteTone(0, freq);
        delay(15);
    }
    for (int freq = 1500; freq >= 700; freq -= 50) {
        ledcWriteTone(0, freq);
        delay(15);
    }
}

void setHazardLights(HazardZone zone) {
    digitalWrite(PIN_LED_GREEN, zone == ZONE_GREEN_SAFE ? HIGH : LOW);
    digitalWrite(PIN_LED_YELLOW, zone == ZONE_YELLOW_CAUTION ? HIGH : LOW);
    digitalWrite(PIN_LED_RED, zone == ZONE_RED_DANGER ? HIGH : LOW);
}

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println(F("======================================================"));
    Serial.println(F("BAIUST CSE 301 - Integrated Design Project"));
    Serial.println(F("ESP32-S3 Rural AI Lightning, Cloud & SIM-GIS Node"));
    Serial.println(F("Team BlackBerry: Sakib (1119003), Ifag, Towhid"));
    Serial.println(F("======================================================"));

    // 1. Initialize GPIOs
    pinMode(PIN_LED_GREEN, OUTPUT);
    pinMode(PIN_LED_YELLOW, OUTPUT);
    pinMode(PIN_LED_RED, OUTPUT);
    pinMode(PIN_GRID_RELAY, OUTPUT);
    pinMode(PIN_BTN_TEST_STORM, INPUT_PULLUP);
    pinMode(PIN_BTN_MUTE_SIREN, INPUT_PULLUP);

    // Initial Safe State: Green Light ON, Relay ON (Normal Mains Connected), Siren OFF
    setHazardLights(ZONE_GREEN_SAFE);
    digitalWrite(PIN_GRID_RELAY, HIGH); // Normally-Closed energized / connected

    // 2. Setup PWM channel for PAM8403 Field Siren Speaker
    ledcSetup(0, 1000, 8); // Channel 0, 1kHz, 8-bit resolution
    ledcAttachPin(PIN_SIREN_PWM, 0);
    ledcWrite(0, 0);

    // 3. Initialize I2C Bus
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 400000);

    // 4. Attach AS3935 Hardware Interrupt
    pinMode(PIN_AS3935_IRQ, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_AS3935_IRQ), onAS3935Interrupt, RISING);

    // 5. Initialize Cellular SIM800L / A7670C
    sim.begin(9600);
    Serial.println(F("[SIM] Cellular Module Initialized."));

    // Default sensor baseline (Winter in Cumilla)
    currentReadings.lux = 650.0f;
    currentReadings.ambientTempC = 22.5f;
    currentReadings.skyTempC = -15.0f; // Clear cold sky
    currentReadings.pressureHPa = 1013.2f;
    currentReadings.pressureDelta = -0.2f;
    currentReadings.humidityPct = 48.0f;
    currentReadings.strikeDetected = false;
    currentReadings.strikeDistanceKm = -1;
    currentReadings.apiCloudCover = 10.0f;
    currentReadings.apiCape = 120.0f;
}

unsigned long lastTelemetryTime = 0;
unsigned long lastAPICallTime = 0;

void loop() {
    // Check Winter Lab Test Button (Simulate live storm GPS switch)
    if (digitalRead(PIN_BTN_TEST_STORM) == LOW) {
        sim.stormSimulationMode = !sim.stormSimulationMode;
        Serial.print(F("[TEST MODE] Storm GPS Switch Toggled. Active = "));
        Serial.println(sim.stormSimulationMode ? "TRUE (Equatorial Storm GPS)" : "FALSE (Cumilla Winter)");
        delay(400); // debounce
    }

    // Handle AS3935 Lightning Interrupt
    if (as3935InterruptFired) {
        as3935InterruptFired = false;
        // In real hardware: read AS3935 INT register 0x03 to verify Lightning vs Disturber vs Noise
        // Simulated response based on spark shocker distance in lab:
        currentReadings.strikeDetected = true;
        // If lab simulation button or recent shocker detected
        currentReadings.strikeDistanceKm = sim.stormSimulationMode ? 8 : 24; 
        Serial.print(F("[AS3935] Lightning Strike RF Event Detected! Distance = "));
        Serial.print(currentReadings.strikeDistanceKm);
        Serial.println(F(" km"));
    }

    // Periodic SIM Weather API Cross-verification (Every 30 seconds)
    if (millis() - lastAPICallTime > 30000 || lastAPICallTime == 0) {
        lastAPICallTime = millis();
        WeatherAPIData api = sim.fetchCloudAndCapeData(23.46f, 91.18f);
        if (api.isValid) {
            currentReadings.apiCloudCover = api.cloudCoverPct;
            currentReadings.apiCape = api.capeIndex;
        }
    }

    // Process 3-Stream Fusion Decision
    float combinedRisk = 0.0f;
    HazardZone zone = fusion.computeHazardZone(currentReadings, combinedRisk);

    // Apply Physical Actuation & Alarms
    setHazardLights(zone);

    if (zone == ZONE_RED_DANGER) {
        // Red Zone Actions:
        // 1. Cut off main power line immediately
        digitalWrite(PIN_GRID_RELAY, LOW); 
        // 2. Sound 3W Loud Evacuation Siren
        playFieldSiren(true);
        // 3. Dispatch Emergency SMS
        static unsigned long lastSMSTime = 0;
        if (millis() - lastSMSTime > 60000) {
            lastSMSTime = millis();
            sim.sendEmergencySMS("+8801700000000", "EMERGENCY: Overhead Lightning <15km detected in BAIUST Sector! Disconnect electronics and seek shelter!");
            Serial.println(F("[SMS] Emergency Alert Dispatched over SIM800L."));
        }
    } else if (zone == ZONE_YELLOW_CAUTION) {
        // Yellow Zone: Mains power stays ON, caution beeps, Yellow Light ON
        digitalWrite(PIN_GRID_RELAY, HIGH);
        ledcWrite(0, 0); // No continuous siren
    } else {
        // Green Zone: Everything safe
        digitalWrite(PIN_GRID_RELAY, HIGH);
        ledcWrite(0, 0);
    }

    // Output Telemetry Stream to Serial (for Lab Video Auditor & Web GIS)
    if (millis() - lastTelemetryTime > 2000) {
        lastTelemetryTime = millis();
        Serial.print(F("CSV_TELEMETRY,"));
        Serial.print(millis()); Serial.print(F(","));
        Serial.print(currentReadings.lux); Serial.print(F(","));
        Serial.print(currentReadings.ambientTempC); Serial.print(F(","));
        Serial.print(currentReadings.skyTempC); Serial.print(F(","));
        Serial.print(currentReadings.pressureHPa); Serial.print(F(","));
        Serial.print(currentReadings.humidityPct); Serial.print(F(","));
        Serial.print(currentReadings.strikeDistanceKm); Serial.print(F(","));
        Serial.print(currentReadings.apiCloudCover); Serial.print(F(","));
        Serial.print(combinedRisk); Serial.print(F(","));
        Serial.println(zone == ZONE_RED_DANGER ? "RED" : (zone == ZONE_YELLOW_CAUTION ? "YELLOW" : "GREEN"));
    }

    // Serial Lab Commands for Professors Viva
    if (Serial.available()) {
        char cmd = Serial.read();
        if (cmd == 'c' || cmd == 'C') {
            // Lab Simulation: Umbrella Cloud Cover
            currentReadings.lux = 12.0f;
            currentReadings.skyTempC = 16.5f; // warm cloud base
            Serial.println(F("[LAB] Simulating Cloud Cover: Lux=12, Sky IR Temp=16.5C"));
        } else if (cmd == 'm' || cmd == 'M') {
            // Lab Simulation: Ultrasonic Mist Maker on DHT22
            currentReadings.humidityPct = 96.0f;
            Serial.println(F("[LAB] Simulating Mist: Humidity=96%"));
        } else if (cmd == 'p' || cmd == 'P') {
            // Lab Simulation: Syringe Pressure Drop on BMP280
            currentReadings.pressureHPa = 991.0f;
            currentReadings.pressureDelta = -4.5f;
            Serial.println(F("[LAB] Simulating Syringe Vacuum: Pressure=991 hPa"));
        } else if (cmd == 's' || cmd == 'S') {
            // Lab Simulation: 400kV Shocker Trigger
            as3935InterruptFired = true;
            Serial.println(F("[LAB] 400kV Spark Event Triggered!"));
        } else if (cmd == 'r' || cmd == 'R') {
            // Reset to winter baseline
            currentReadings.lux = 650.0f;
            currentReadings.skyTempC = -15.0f;
            currentReadings.pressureHPa = 1013.2f;
            currentReadings.pressureDelta = -0.1f;
            currentReadings.humidityPct = 48.0f;
            currentReadings.strikeDetected = false;
            currentReadings.strikeDistanceKm = -1;
            Serial.println(F("[LAB] Baseline Reset to Clear Winter Sky."));
        }
    }
}
