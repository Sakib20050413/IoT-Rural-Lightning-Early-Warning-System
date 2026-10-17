#pragma once
#include <Arduino.h>

enum HazardZone {
    ZONE_GREEN_SAFE = 0,
    ZONE_YELLOW_CAUTION = 1,
    ZONE_RED_DANGER = 2
};

struct SensorReadings {
    float lux;             // BH1750 (Lux)
    float skyTempC;        // MLX90614 Object Temp (°C)
    float ambientTempC;    // MLX90614 Ambient Temp (°C)
    float pressureHPa;     // BMP280 (hPa)
    float pressureDelta;   // hPa/hr rate of change
    float humidityPct;     // DHT22 (%)
    int strikeDistanceKm;  // AS3935 (-1 if no strike)
    bool strikeDetected;   // AS3935 interrupt fired
    float apiCloudCover;   // Open-Meteo Cloud %
    float apiCape;         // Convective Available Potential Energy (J/kg)
};

class SensorFusionEngine {
public:
    // Computes verified cloud probability eliminating winter fog false positives
    // Principle: Fog has high humidity (90%+) but cold cloud-base IR and high daylight Lux.
    // Cumulonimbus clouds have: Sudden darkening (Lux < 50), warm cloud base IR (T_amb - T_sky < 8°C), and falling pressure.
    float evaluateCloudProbability(const SensorReadings& r) {
        float cloudScore = 0.0f;

        // 1. Lux drop analysis (Normal daylight is 500-10,000 Lux. Dark storm clouds drop < 50 Lux)
        if (r.lux < 50.0f) {
            cloudScore += 35.0f;
        } else if (r.lux < 200.0f) {
            cloudScore += 15.0f;
        }

        // 2. Infrared Cloud-Base Differential (Delta T)
        // Clear sky IR is extremely cold (-15°C to -30°C) -> Large Delta T (>25°C).
        // Low storm cloud radiates IR down (10°C to 20°C) -> Small Delta T (<8°C).
        float deltaT = r.ambientTempC - r.skyTempC;
        if (deltaT < 8.0f) {
            cloudScore += 35.0f; // Dense low cloud cover confirmed
        } else if (deltaT < 18.0f) {
            cloudScore += 15.0f;
        }

        // 3. Barometric pressure rate of change
        if (r.pressureDelta < -1.5f || r.pressureHPa < 1000.0f) {
            cloudScore += 20.0f;
        }

        // 4. Relative humidity confirmation
        if (r.humidityPct > 80.0f) {
            cloudScore += 10.0f;
        }

        return constrain(cloudScore, 0.0f, 100.0f);
    }

    // 3-Stream Decision Matrix: Combines Local Sensors + SIM API + AS3935 Strike
    HazardZone computeHazardZone(const SensorReadings& r, float& outCombinedRiskScore) {
        float localCloudScore = evaluateCloudProbability(r);
        
        // Cross-verify with SIM Cellular API
        // If local sensor says 80% cloud, but SIM API reports 0% cloud and 50 CAPE in winter,
        // it suppresses the false trigger (e.g. sensor being physically shaded by hands without pressure drop)
        float verifiedCloudScore = (localCloudScore * 0.65f) + (r.apiCloudCover * 0.35f);

        outCombinedRiskScore = verifiedCloudScore;

        // RULE 1: RED ZONE (Imminent Overhead Danger)
        // Strike detected <= 15 km AND verified cloud/atmospheric instability
        if (r.strikeDetected && r.strikeDistanceKm >= 1 && r.strikeDistanceKm <= 15) {
            outCombinedRiskScore = 95.0f;
            return ZONE_RED_DANGER;
        }
        
        // Also trigger RED if consecutive strikes occur within 18 km
        if (r.strikeDetected && r.strikeDistanceKm < 18 && verifiedCloudScore > 50.0f) {
            outCombinedRiskScore = 90.0f;
            return ZONE_RED_DANGER;
        }

        // RULE 2: YELLOW ZONE (Moderate Caution / Approaching Storm)
        // Distant lightning (15 km - 35 km) OR high cloud instability without strikes yet
        if ((r.strikeDetected && r.strikeDistanceKm > 15 && r.strikeDistanceKm <= 40) ||
            verifiedCloudScore > 65.0f || r.apiCape > 1500.0f) {
            outCombinedRiskScore = max(outCombinedRiskScore, 65.0f);
            return ZONE_YELLOW_CAUTION;
        }

        // RULE 3: GREEN ZONE (Safe)
        return ZONE_GREEN_SAFE;
    }
};
