[![Architecture diagram](https://gitdiagram.com/diagram-badge.svg)](https://gitdiagram.com/sakib20050413/iot-rural-lightning-early-warning-system?utm_source=readme&utm_medium=badge)


⚡ IoT Rural Lightning Early Warning System

An AI-powered edge intelligence & GIS surveillance network designed for real-time lightning detection, false-alarm mitigation, and automated community warning in rural environments.

ESP32-S3




License

🌟 Executive Overview

Lightning strikes pose severe threats to rural agricultural workers and off-grid communities where traditional meteorological radar coverage is limited or delayed.

The IoT Rural Lightning Early Warning System bridges this life-safety gap through an integrated, three-tier architecture:

Edge-Sensing Tier: Dual-mode electro-magnetic and atmospheric sensing via AS3935 Franklin Lightning Sensor and multi-sensor telemetry running on an ESP32-S3 microcontroller.

TinyML Intelligence Tier: On-device micro-inference filtering ambient interference (high-voltage powerlines, motors, radio noise) to eliminate costly false positives.

Dual-Alert & Cloud Tier: Autonomous dual dispatch triggering an immediate high-decibel field siren locally while relaying geographic warning packets via cellular SIM (GSM/GPRS) to a central Web-GIS surveillance dashboard.

🏗️ System Architecture & Data Flow

flowchart TD
    subgraph SENSING["1. Edge Data Acquisition"]
        A1[AS3935 Franklin Detector] -->|I2C / IRQ| ESP[ESP32-S3 Dual-Core SoC]
        A2[Atmospheric Sensors] -->|Analog / ADC| ESP
        A3[Electromagnetic Monitor] -->|Telemetry| ESP
    end

    subgraph PROCESSING["2. Edge AI & Sensor Fusion"]
        ESP --> FUSION[Sensor Fusion Engine]
        FUSION --> TINYML{TinyML Filter}
        TINYML -->|Man-Made Noise| DISCARD[Discard False Alarm]
        TINYML -->|Confirmed Lightning Strike| DISPATCH[Alert Dispatcher]
    end

    subgraph ALERTING["3. Dual-Channel Alerting"]
        DISPATCH -->|PWM Hardware Drive| SIREN[Field Siren / Warning Beacon]
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


🚀 Key Features

⚡ Ultra-Low Latency Edge Detection: Millisecond-level interrupt handling via dedicated hardware ISR on ESP32-S3.

🧠 Noise Elimination via TinyML: On-device neural network and heuristic fusion distinguishing legitimate cloud-to-ground strikes from industrial electromagnetic spikes.

📡 Autonomous Cellular Telemetry: Resilient SIM connection routines with automatic fallback and self-healing network reconnects.

🚨 Zero-Cloud Fail-Safe Siren: Local siren operates completely off-grid without requiring internet connectivity.

🗺️ Interactive Web GIS Dashboard: Live geospatial map visualizing danger zones (Safe, Caution, Critical strike radius), real-time signal RSSI, and incident event logs.

📁 Repository Structure

IoT-Rural-Lightning-Early-Warning-System/
├── 3Stream_TinyML_SIM_Siren_Pipeline.png            # Visual system pipeline diagram
├── Project1_ESP32S3_SIM_Cloud_Siren_3D_Poster.jpg   # 3D project visualization poster
├── firmware/                                       # Production PlatformIO firmware
│   ├── platformio.ini                              # PlatformIO board & dependency configurations
│   ├── include/                                    # Hardware definitions & abstraction headers
│   │   ├── pinout.h                                # GPIO pin mapping for ESP32-S3
│   │   ├── sensor_fusion.h                         # Sensor fusion & strike classification logic
│   │   └── sim_manager.h                           # Cellular AT command & network manager
│   └── src/
│       └── main.cpp                                # Core runtime loop & ISR handlers
├── firmware_esp32s3/                               # Standalone Arduino IDE source
│   └── project1_lightning_sim_tinyml.ino           # Single-file development sketch
├── web_gis/                                        # Core Web GIS surveillance client
│   └── index.html                                  # Leaflet.js real-time incident dashboard
└── web_gis_full_website/                           # Comprehensive portal & analytics
    └── index.html                                  # Extended multi-zone monitoring dashboard


🔌 Hardware Pinout Mapping

Component

ESP32-S3 Pin

Protocol / Mode

Description

AS3935 SDA

GPIO 21

I2C Data

Lightning detector bus communication

AS3935 SCL

GPIO 22

I2C Clock

Lightning detector bus clock

AS3935 IRQ

GPIO 19

Hardware Interrupt

Triggered on strike/disturber detection

SIM RX / TX

GPIO 16 / 17

Hardware UART

AT communication with SIM cellular module

Field Siren

GPIO 25

PWM Output

High-frequency warning tone generator

Status LED

GPIO 2

Digital Output

System heartbeat & cellular connection status

🛠️ Getting Started

1. Firmware Setup (PlatformIO)

Clone the repository:

git clone https://github.com/sakib20050413/iot-rural-lightning-early-warning-system.git
cd iot-rural-lightning-early-warning-system/firmware


Open the project inside VS Code with the PlatformIO extension installed.

Verify your pin connections in include/pinout.h.

Build and flash the firmware:

pio run --target upload


2. Live Web GIS Surveillance Dashboard

The web surveillance interface runs directly on any modern browser:

Navigate to web_gis/index.html or web_gis_full_website/index.html.

Open the file in your browser, or host it with GitHub Pages:

Go to Settings > Pages > Branch: main > Save.

Access your live dashboard URL directly from anywhere.

📄 License

This project is open-source and licensed under the MIT License.
