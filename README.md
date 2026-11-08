[![Architecture diagram](https://gitdiagram.com/diagram-badge.svg)](https://gitdiagram.com/sakib20050413/iot-rural-lightning-early-warning-system?utm_source=readme&utm_medium=badge)


# ⚡ IoT Rural Lightning Early Warning System

> **An AI-powered edge intelligence & GIS surveillance network designed for real-time lightning detection, false-alarm mitigation, and automated community warning in rural environments.**

---

## 📌 Project Visuals

### 🎯 3D System Design & Hardware Overview
[./Project1_ESP32S3_SIM_Cloud_Siren_3D_Poster.jpg]

### 📊 End-to-End Operational Pipeline
[./3Stream_TinyML_SIM_Siren_Pipeline.png]

---

## 🌟 Executive Overview

Lightning strikes pose severe threats to rural agricultural workers and off-grid communities where traditional meteorological radar coverage is limited or delayed.

The **IoT Rural Lightning Early Warning System** bridges this life-safety gap through an integrated, three-tier architecture:

1. **Edge-Sensing Tier:** Dual-mode electro-magnetic and atmospheric sensing via the **AS3935 Franklin Lightning Sensor** running on an **ESP32-S3** microcontroller.
2. **TinyML Intelligence Tier:** On-device micro-inference filtering ambient interference (high-voltage powerlines, motors, industrial RF noise) to eliminate costly false alarms.
3. **Dual-Alert & Cloud Tier:** Autonomous dual dispatch triggering an immediate high-decibel field siren locally while relaying geographic warning packets via cellular SIM (GSM/GPRS) to a central **Web-GIS surveillance dashboard**.

> [!IMPORTANT]
> **Zero-Cloud Fail-Safe:** The local audio siren triggers instantaneously via hardware PWM even if cellular network coverage is completely unavailable.

---

## 🏗️ System Architecture & Workflow

```mermaid
flowchart TD
    subgraph SENSING["1. Edge Data Acquisition"]
        A1[AS3935 Franklin Detector] -->|I2C / Hardware IRQ| ESP[ESP32-S3 Dual-Core SoC]
        A2[Atmospheric Sensors] -->|Analog Telemetry| ESP
        A3[Electromagnetic Monitor] -->|RF Telemetry| ESP
    end

    subgraph PROCESSING["2. Edge AI & Sensor Fusion"]
        ESP --> FUSION[Sensor Fusion Engine]
        FUSION --> TINYML{TinyML Filter}
        TINYML -->|Man-Made RF Noise| DISCARD[Discard False Alarm]
        TINYML -->|Confirmed Lightning Strike| DISPATCH[Alert Dispatcher]
    end

    subgraph ALERTING["3. Dual-Channel Alerting"]
        DISPATCH -->|Hardware PWM Drive| SIREN[Field Siren / Warning Beacon]
        DISPATCH -->|AT Command Pipeline| SIM[Cellular SIM Module]
    end

    subgraph SURVEILLANCE["4. Central Cloud & GIS"]
        SIM -->|Cellular Network / HTTP| CLOUD[Cloud Surveillance Server]
        CLOUD --> DASHBOARD[Live Leaflet Web-GIS Surveillance Dashboard]
    end

    style SENSING fill:#111827,stroke:#3b82f6,stroke-width:2px,color:#fff
    style PROCESSING fill:#111827,stroke:#f59e0b,stroke-width:2px,color:#fff
    style ALERTING fill:#111827,stroke:#ef4444,stroke-width:2px,color:#fff
    style SURVEILLANCE fill:#111827,stroke:#10b981,stroke-width:2px,color:#fff
