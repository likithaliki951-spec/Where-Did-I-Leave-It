# TraceTag — Comprehensive Setup & Development Guide

This guide walks you through setting up the development environment, compiling and uploading the ESP32 firmware, and running the Flutter mobile application.

---

## 1. Prerequisites

### Hardware
- 1× ESP32 Development Board (ESP32 DevKit V1, NodeMCU-32S, or WROOM-32)
- 1× MPU6050 6-Axis Accelerometer/Gyroscope Module (GY-521)
- 4× Female-to-Female jumper wires
- 1× Micro-USB / USB-C data cable (ensure it supports data transfer, not charge-only)
- 1× Optional 3.3V Active Buzzer or external LED

### Software
- **ESP32 Toolchain:** Arduino IDE 2.x or `arduino-cli`
  - Bundled CLI location: `C:\Users\<User>\AppData\Local\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe`
- **ESP32 Core:** Espressif Systems `esp32` package (v3.0.0 or higher)
- **Flutter SDK:** Flutter 3.24+ (Dart 3.5+)
- **Python:** Python 3.10+ (for automated test suite)

---

## 2. Firmware Compilation & Flashing

### Option A: Using `arduino-cli` (Command Line)

1. **Verify board recognition:**
   ```powershell
   arduino-cli board list
   ```
   Note the COM port assigned to your ESP32 (e.g., `COM3`, `COM5`).

2. **Step 1 — Verify Hardware Wiring via I2C Diagnostic Scanner:**
   Compile and upload the I2C diagnostic sketch:
   ```powershell
   arduino-cli compile --fqbn esp32:esp32:esp32 tests/i2c_scan
   arduino-cli upload -p COM5 --fqbn esp32:esp32:esp32 tests/i2c_scan
   ```
   Open the serial monitor at `115200` baud:
   ```powershell
   arduino-cli monitor -p COM5 -c baudrate=115200
   ```
   You should see: `Device found at address 0x68 <-- MPU6050 (expected!)`.

3. **Step 2 — Flash Main TraceTag Firmware:**
   ```powershell
   arduino-cli compile --fqbn esp32:esp32:esp32 firmware/TraceTag_Main
   arduino-cli upload -p COM5 --fqbn esp32:esp32:esp32 firmware/TraceTag_Main
   ```

4. **Alternative: Direct binary flashing with esptool:**
   Pre-compiled binaries are exported in `firmware/build/`:
   ```powershell
   python -m esptool --chip esp32 --port COM5 --baud 921600 write_flash 0x0 firmware/build/TraceTag_Main.ino.merged.bin
   ```

### Option B: Using Arduino IDE 2

1. Launch Arduino IDE.
2. Open `firmware/TraceTag_Main/TraceTag_Main.ino`.
3. Under **Tools → Board → esp32**, select **ESP32 Dev Module**.
4. Under **Tools → Port**, select your active COM port.
5. Click **Verify (✓)** to compile.
6. Click **Upload (➔)** to flash to the ESP32.

---

## 3. Flutter Mobile Application Setup

### 3.1 Dependency Resolution & Asset Verification
Navigate to the root directory and run:
```powershell
flutter pub get
```

### 3.2 Running the Application
Connect an Android smartphone with USB Debugging enabled, or start an Android Emulator:
```powershell
flutter devices
flutter run
```

### 3.3 Runtime Permissions
When launching the app for the first time:
- Allow **Nearby Devices / Bluetooth** permission.
- Allow **Location** permission (required by Android OS for BLE scanning).

---

## 4. End-to-End Operational Verification

1. Power on the ESP32 board.
2. Open the TraceTag application on your mobile device.
3. Tap **Scan ESP32**.
4. Locate the device named `WhereDidILeaveIt` and tap **Connect**.
5. Once connected, the app status changes to `Device Ready`.
6. Tap **Find Object**:
   - The ESP32 onboard LED (and buzzer) will begin pulsing at 4 Hz.
7. Gently lift or shake the MPU6050 sensor:
   - The ESP32 detects the acceleration deviation.
   - The Flutter application immediately updates to `Object Found 🔍`.
8. Tap **Stop**:
   - The alerts cease and status returns to `Device Ready`.
9. Tap **Disconnect** when finished.

---

## 5. Troubleshooting Matrix

| Symptom | Cause | Solution |
| :--- | :--- | :--- |
| `[ERROR] MPU6050 not found` | Faulty wiring or loose jumper | Check GPIO 21 (SDA) and GPIO 22 (SCL). Ensure 3.3V power is used. |
| Upload fails with `A fatal error occurred: Failed to connect to ESP32` | ESP32 bootloader handshake timed out | Hold down the **BOOT** button on the ESP32 board while the upload command starts connecting. |
| Device `WhereDidILeaveIt` not appearing in scan | BLE advertising not started or Bluetooth disabled on phone | Ensure phone Bluetooth and GPS are toggled ON. Check ESP32 Serial Monitor to ensure `[BLE] Advertising started`. |
| `Unable to load asset: assets/logo.png` | Asset path mismatch | Verified fixed: `assets/logo.png` is placed at root and declared in `pubspec.yaml`. Run `flutter pub get`. |
