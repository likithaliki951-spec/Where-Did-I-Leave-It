# TraceTag — System Architecture & Protocol Specification

## 1. Architectural Overview

TraceTag is an IoT-enabled smart object tracking and physical locator ecosystem. It bridges an ESP32-based hardware tag with an interactive Flutter mobile client over low-latency Bluetooth Low Energy (BLE 4.2 / 5.0).

```mermaid
graph TD
    subgraph "Flutter Mobile Application"
        UI[Material 3 UI Layer]
        BLOC[State & Lifecycle Controller]
        FBP[flutter_blue_plus BLE Client]
        PERM[Permission Handler Subsystem]
        UI --> BLOC
        BLOC --> FBP
        BLOC --> PERM
    end

    subgraph "Wireless Interface (BLE GATT)"
        ADV[BLE Advertisements: WhereDidILeaveIt]
        GATT[GATT Service: 12345678-1234-1234-1234-1234567890AB]
        CHAR[GATT Characteristic: ABCDEFAB-1234-5678-1234-ABCDEF123456]
    end

    subgraph "ESP32 Firmware System"
        BLE_MGR[BLE GATT Server & Callbacks]
        STATE_MGR[Tag State Machine Controller]
        MOTION[MotionDetector Engine]
        IMU_DRV[MPU6050 Direct I2C Driver]
        HW_I2C[Hardware I2C: SDA=21, SCL=22]
        HW_ALERT[Visual & Audio Alerts: GPIO 2 & 23]

        BLE_MGR --> STATE_MGR
        STATE_MGR --> MOTION
        STATE_MGR --> HW_ALERT
        MOTION --> IMU_DRV
        IMU_DRV --> HW_I2C
    end

    FBP <== Wireless BLE Link ==> ADV
    FBP <== Read / Write / Notify ==> CHAR
```

---

## 2. Firmware Architecture Layers

### 2.1 Hardware Abstraction & Driver Layer (`MPU6050Handler`)
- **Direct I2C Register Control:** Built upon Arduino's native `<Wire.h>` library without external library dependencies (e.g. eliminating Adafruit BusIO / Unified Sensor dependency constraints).
- **Auto-Detection:** Probes primary address `0x68` and automatically falls back to `0x69` if AD0 is pulled high.
- **Power Management:** Automatically awakens sensor from default sleep state (`PWR_MGMT_1` = `0x00`).
- **Signal Filtering:** Configures Digital Low Pass Filter (DLPF) at register `0x1A` with ~44 Hz bandwidth to suppress high-frequency vibrational noise.
- **Burst Sampling:** Reads 14 sequential registers starting from `0x3B` (`ACCEL_XOUT_H`) in a single I2C transaction, converting 16-bit raw signed integers into SI units ($m/s^2$, $rad/s$, $^\circ C$).

### 2.2 Processing Layer (`MotionDetector`)
- **Acceleration Vector Magnitude:**
  $$\|\vec{a}\| = \sqrt{a_x^2 + a_y^2 + a_z^2}$$
- **Dynamic Baseline Compensation:** Continuously filters ambient gravitational baseline ($\sim 9.81\ m/s^2$) using an exponential smoothing filter when quiescent:
  $$B_{t} = 0.96 \cdot B_{t-1} + 0.04 \cdot \|\vec{a}\|$$
- **Angular Velocity Vector Magnitude:**
  $$\|\vec{\omega}\| = \sqrt{\omega_x^2 + \omega_y^2 + \omega_z^2}$$
- **Dual-Threshold Detection & Debounce:** Motion is declared true when deviation $|\|\vec{a}\| - B_t| > 2.0\ m/s^2$ OR $\|\vec{\omega}\| > 0.75\ rad/s$ for at least 2 consecutive polling cycles (`TRIGGER_CONFIRM_COUNT = 2`).

### 2.3 Communication Layer (BLE GATT Server)
- **Advertised Name:** `WhereDidILeaveIt`
- **Primary Service UUID:** `12345678-1234-1234-1234-1234567890AB`
- **Primary Characteristic UUID:** `ABCDEFAB-1234-5678-1234-ABCDEF123456`
- **Characteristic Permissions:** `READ`, `WRITE`, `NOTIFY`
- **Descriptor:** `BLE2902` Client Characteristic Configuration Descriptor (CCCD) enabled to support asynchronous notifications to Flutter.

---

## 3. Finite State Machine (FSM)

```mermaid
stateDiagram-v2
    [*] --> STANDBY: Boot / Init Complete
    
    STANDBY --> STANDBY: Normal Polling / Heartbeat
    STANDBY --> FINDING: Write "FIND_OBJECT"
    
    FINDING --> FINDING: Pulse LED/Buzzer (4 Hz)
    FINDING --> FOUND: Motion Detected (Pick up / Displace)
    FINDING --> STANDBY: Write "STOP_FIND" (Notify "READY")
    
    FOUND --> FOUND: Double-flash Alert
    FOUND --> STANDBY: Write "STOP_FIND" (Notify "READY")
```

| State | Indicators | Sensor State | BLE Status |
| :--- | :--- | :--- | :--- |
| **STANDBY** | LED Off, Buzzer Off | Passive baseline tracking | Connected or Advertising |
| **FINDING** | LED & Buzzer pulsing at 4 Hz | Detection Armed | Notifies `READY` on entry |
| **FOUND** | Double-flash LED alert pattern | Motion confirmed | Notifies `OBJECT_FOUND` immediately |

---

## 4. End-to-End Sequence Diagram

```mermaid
sequenceDiagram
    autonumber
    actor User
    participant App as Flutter Mobile App
    participant BLE as BLE GATT Stack
    participant ESP as ESP32 Controller
    participant IMU as MPU6050 Sensor

    User->>App: Tap "Scan ESP32"
    App->>BLE: Start BLE Discovery (10s timeout)
    BLE-->>App: Discovered "WhereDidILeaveIt"
    App->>User: Display device in list

    User->>App: Tap "Connect"
    App->>BLE: Connect(timeout: 15s)
    App->>BLE: Discover Services & Characteristics
    App->>BLE: Enable Characteristic Notifications (CCCD 0x2902)
    ESP->>BLE: Notify "READY"
    BLE-->>App: Receive "READY"
    App->>User: UI displays "Device Ready"

    User->>App: Tap "Find Object"
    App->>BLE: Write "FIND_OBJECT" (withResponse: false)
    BLE->>ESP: onWrite("FIND_OBJECT")
    ESP->>ESP: Enter STATE_FINDING (Arm motion, pulse LED)

    loop 20 Hz Polling
        ESP->>IMU: Burst read (14 bytes)
        IMU-->>ESP: Accel & Gyro vectors
    end

    Note over User,IMU: Tagged item is picked up or physically disturbed
    ESP->>ESP: Motion detected (Threshold exceeded)
    ESP->>BLE: Notify "OBJECT_FOUND"
    BLE-->>App: Receive "OBJECT_FOUND"
    App->>User: UI updates to "Object Found 🔍"

    User->>App: Tap "Stop"
    App->>BLE: Write "STOP_FIND"
    BLE->>ESP: onWrite("STOP_FIND")
    ESP->>ESP: Enter STATE_STANDBY (Turn off alerts)
    ESP->>BLE: Notify "READY"
    BLE-->>App: Receive "READY"
    App->>User: UI updates to "Device Ready"
```

---

## 5. Flutter Application Architecture

- **Entry Point:** `lib/main.dart`
- **SDK Compatibility:** Dart `>=3.0.0 <4.0.0`
- **Core Dependencies:**
  - `flutter_blue_plus: ^1.35.5` — BLE client abstraction
  - `permission_handler: ^11.4.0` — Runtime Bluetooth and Location permissions
  - `cupertino_icons: ^1.0.8` — Cross-platform iconography
- **Asset Configuration:**
  - Declared: `assets/logo.png`
  - Canonical location: `<project_root>/assets/logo.png`
- **Error Handling:** Graceful retry on failed connections, proactive stream cancellation during dispose, and UI state resilience against asynchronous unmounted callbacks.
