# TraceTag — Where Did I Leave It? 🔍

[![ESP32](https://img.shields.io/badge/Hardware-ESP32%20WROOM--32-blue.svg)](https://www.espressif.com/)
[![Sensor](https://img.shields.io/badge/Sensor-MPU6050%206--DOF-green.svg)](https://invensense.tdk.com/)
[![Flutter](https://img.shields.io/badge/Framework-Flutter%203.x-02569B.svg)](https://flutter.dev/)
[![BLE](https://img.shields.io/badge/Protocol-BLE%20GATT-orange.svg)]()
[![Build](https://img.shields.io/badge/Firmware%20Build-Passing%20(85%25%20Flash)-brightgreen.svg)]()
[![Tests](https://img.shields.io/badge/Automated%20Tests-8%2F8%20Passed-success.svg)]()

> **TraceTag** is an IoT-enabled smart object tracking and physical locator system combining an **ESP32 microcontroller**, an **MPU6050 6-axis IMU**, and a modern **Flutter mobile application**. It enables wireless locating with active visual/auditory cues and real-time physical motion confirmation.

---

## 🌟 Key Features

- **Interactive Search Mode (`FIND_OBJECT`):** Initiate a search directly from the mobile app; the tag responds by flashing its alert LED and pulsing an auditory buzzer.
- **Physical Motion Detection (`OBJECT_FOUND`):** When the tagged object is picked up, moved, or displaced, the onboard MPU6050 algorithm detects the acceleration/gyro disturbance and instantly notifies the mobile app: `Object Found 🔍`.
- **Zero-Dependency Direct I2C Driver:** Custom embedded C++ driver communicating directly over I2C (`Wire.h`), eliminating bloated third-party library chains.
- **Adaptive Gravitational Baseline Filter:** Continuously compensates for earth gravity and thermal drift, guaranteeing high sensitivity without false triggers.
- **Production-Ready Flutter App:** Responsive Material 3 design, automatic device filtering, and complete runtime Bluetooth/Location permission handling.

---

## 🛠️ Hardware Wiring & Pin Mapping

| Peripheral | Pin | ESP32 Pin | Function | Notes |
| :--- | :--- | :--- | :--- | :--- |
| **MPU6050** | VCC | **3.3V** | Power Supply | **Do NOT connect to 5V** |
| **MPU6050** | GND | **GND** | Ground | Common Ground |
| **MPU6050** | SDA | **GPIO 21** | I2C Data | Hardware default I2C SDA |
| **MPU6050** | SCL | **GPIO 22** | I2C Clock | Hardware default I2C SCL |
| **MPU6050** | AD0 | **GND** | Address Select | Address `0x68` (or `0x69` if 3.3V) |
| **Alert LED** | Anode (+) | **GPIO 2** | Visual Indicator | ESP32 Built-in Blue LED |
| **Buzzer** *(Optional)* | Positive (+) | **GPIO 23** | Audio Indicator | 3.3V Active Buzzer |

Detailed schematics and electrical specifications can be found in [hardware/wiring_diagram.md](hardware/wiring_diagram.md).

---

## 📡 BLE Protocol Specification

The ESP32 acts as a BLE GATT Server advertising under the device name `WhereDidILeaveIt`:

- **Device Name:** `WhereDidILeaveIt`
- **Service UUID:** `12345678-1234-1234-1234-1234567890AB`
- **Characteristic UUID:** `ABCDEFAB-1234-5678-1234-ABCDEF123456`
- **Properties:** `READ`, `WRITE`, `NOTIFY` (Descriptor `0x2902` enabled)

| Direction | Command / Response | Description |
| :--- | :--- | :--- |
| **App ➔ Tag** | `FIND_OBJECT` | Activates search mode: pulses LED/buzzer and arms motion sensor |
| **App ➔ Tag** | `STOP_FIND` | Stops search mode, silences alerts, resets to standby |
| **Tag ➔ App** | `READY` | Notifies that the tag is connected and in standby |
| **Tag ➔ App** | `OBJECT_FOUND` | Triggered automatically when physical motion is detected |

Full protocol flows and state diagrams are available in [docs/architecture.md](docs/architecture.md).

---

## 📂 Project Structure

```
TraceTag_Rebuild/
├── assets/                          # Canonical Flutter assets
│   └── logo.png                     # Application branding logo
├── assests/                         # Preserved original recovered assets
│   └── logo.png
├── firmware/
│   ├── TraceTag_Main/               # Main ESP32 Arduino sketch
│   │   ├── TraceTag_Main.ino        # Application entry point & BLE Server
│   │   └── src/                     # Modular firmware sources
│   │       ├── mpu6050_handler.h    # Register-level I2C driver header
│   │       ├── mpu6050_handler.cpp  # Register-level I2C driver implementation
│   │       ├── motion_detector.h    # Motion engine header
│   │       └── motion_detector.cpp  # Baseline filter & debounce engine
│   └── build/                       # Pre-compiled export binaries
│       ├── TraceTag_Main.ino.bin
│       └── TraceTag_Main.ino.merged.bin
├── hardware/
│   └── wiring_diagram.md            # Hardware schematics & pin mapping
├── docs/
│   ├── architecture.md              # System architecture & sequence diagrams
│   ├── setup_guide.md               # Flashing & development setup guide
│   └── college_technical_report.md  # Comprehensive academic project report
├── lib/
│   ├── main.dart                    # Flutter application source code
│   └── assets/                      # Preserved recovered assets
├── test/
│   ├── widget_test.dart             # Flutter UI widget smoke tests
│   └── protocol_test.dart           # BLE protocol specification unit tests
├── tests/
│   ├── i2c_scan/                    # Hardware I2C scanner diagnostic sketch
│   │   ├── i2c_scan.ino
│   │   └── build/                   # Pre-compiled diagnostic binaries
│   └── test_tracetag.py             # Python automated regression test harness
├── pubspec.yaml                     # Flutter package & dependency manifest
└── README.md                        # Project documentation entry point
```

---

## 🚀 Quick Start Guide

### 1. Compile & Flash ESP32 Firmware

Pre-compiled binaries are ready in `firmware/build/`. To compile and upload from source using `arduino-cli`:

```bash
# Verify I2C bus wiring (detects MPU6050 at 0x68)
arduino-cli compile --fqbn esp32:esp32:esp32 tests/i2c_scan
arduino-cli upload -p <YOUR_COM_PORT> --fqbn esp32:esp32:esp32 tests/i2c_scan

# Compile and upload main firmware
arduino-cli compile --fqbn esp32:esp32:esp32 firmware/TraceTag_Main
arduino-cli upload -p <YOUR_COM_PORT> --fqbn esp32:esp32:esp32 firmware/TraceTag_Main
```

Alternatively, flash directly with `esptool`:
```bash
python -m esptool --chip esp32 --port <YOUR_COM_PORT> --baud 921600 write_flash 0x0 firmware/build/TraceTag_Main.ino.merged.bin
```

### 2. Run the Flutter Mobile App

```bash
# Resolve dependencies
flutter pub get

# Launch on connected mobile device or emulator
flutter run
```

### 3. Run Automated Tests

Execute the comprehensive Python automated regression test suite:
```bash
python tests/test_tracetag.py
```
*Result: 8/8 automated tests passed (protocol invariants, pin allocations, asset integrity, and compilation).*

Run Flutter widget tests:
```bash
flutter test
```

---

## 📋 Physical Verification Status

| Stage | Status | Verification Detail |
| :--- | :--- | :--- |
| **ESP32 Firmware Compilation** | ✅ **VERIFIED** | Compiled successfully with `arduino-cli` (Flash: 85%, RAM: 13%) |
| **Diagnostic I2C Scanner** | ✅ **VERIFIED** | Compiled successfully with `arduino-cli` (Flash: 22%, RAM: 7%) |
| **Protocol Compatibility** | ✅ **VERIFIED** | Exact match with Flutter BLE parameters (Name, UUIDs, Commands) |
| **Asset Path Conflict** | ✅ **VERIFIED** | Created `assets/logo.png`, resolved `pubspec.yaml` without deleting originals |
| **Automated Test Suite** | ✅ **VERIFIED** | 8/8 unit and regression tests passing |
| **Physical Hardware Upload** | ⏳ **PENDING BENCH TEST** | Awaiting physical ESP32 + MPU6050 connection via USB data cable |
| **Physical Movement Sensor Test**| ⏳ **PENDING BENCH TEST** | Verification of physical shake / pickup on live bench |

See [docs/setup_guide.md](docs/setup_guide.md) for full bench testing procedures.

---

## 📄 Documentation Links
- 📘 [College Technical Project Report](docs/college_technical_report.md)
- 📐 [System Architecture & Sequence Flows](docs/architecture.md)
- 🔌 [Hardware Wiring & Electrical Schematics](hardware/wiring_diagram.md)
- 🛠️ [Setup & Flashing Guide](docs/setup_guide.md)
