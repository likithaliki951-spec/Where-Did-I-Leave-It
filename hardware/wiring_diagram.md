# TraceTag — Hardware Wiring & Interfacing Reference Guide

## 1. System Overview
TraceTag is an ultra-compact, low-power BLE object tracking tag composed of an ESP32 microcontroller interfaced with an MPU6050 6-axis Inertial Measurement Unit (IMU). The system integrates motion-sensing algorithms and visual/auditory alert transducers.

---

## 2. Pin Mapping Table

| Peripheral | Peripheral Pin | ESP32 Pin | Function | Notes |
| :--- | :--- | :--- | :--- | :--- |
| **MPU6050** | VCC | **3.3V** | Power Supply | **Do NOT connect to 5V / Vin** |
| **MPU6050** | GND | **GND** | Common Ground | Reference ground |
| **MPU6050** | SDA | **GPIO 21** | I2C Serial Data | Hardware default I2C SDA |
| **MPU6050** | SCL | **GPIO 22** | I2C Serial Clock | Hardware default I2C SCL |
| **MPU6050** | AD0 | **GND** | I2C Address Select | Address `0x68` (tie to 3.3V for `0x69`) |
| **MPU6050** | INT | *NC / Floating* | Interrupt | Not required (polled at 20 Hz) |
| **Alert LED** | Anode (+) | **GPIO 2** | Visual Alert | Built-in Blue LED on ESP32 DevKit |
| **Alert LED** | Cathode (-) | **GND** | Ground | Internal on-board connection |
| **Buzzer** *(Optional)* | Positive (+) | **GPIO 23** | Auditory Alert | 3.3V Active Buzzer or NPN driver |
| **Buzzer** *(Optional)* | Negative (-) | **GND** | Ground | Common Ground |

---

## 3. Schematic Wiring Diagram

```
           +---------------------------------------+
           |           ESP32 Dev Module            |
           |                                       |
           |  [ 3.3V ] ---------+                  |
           |  [ GND  ] -----+   |                  |
           |                |   |                  |
           |  [GPIO 21] ----+---|-------+          |
           |  (SDA)         |   |       |          |
           |  [GPIO 22] ----+---|---+   |          |
           |  (SCL)         |   |   |   |          |
           |                |   |   |   |          |
           |  [GPIO 2 ] --> (Built-in Blue LED)    |
           |                                       |
           |  [GPIO 23] ----+                      |
           +----------------|----------------------+
                            |
                            |   +-----------------------+
                            |   |   MPU6050 IMU Breakout|
                            |   |                       |
                            +-> | VCC   (3.3V Power)    |
                            +-> | GND   (Common Ground) |
                            +-> | SCL   (GPIO 22)       |
                            +-> | SDA   (GPIO 21)       |
                                | AD0   (GND -> 0x68)   |
                                | INT   (Unconnected)   |
                                +-----------------------+
                            
         [Optional Buzzer]
         GPIO 23 -------- (+) [Active Buzzer] (-) -------- GND
```

---

## 4. Electrical Specifications & Considerations

### 4.1 I2C Bus Voltage & Pull-Ups
- **Voltage Levels:** The ESP32 operates at 3.3V CMOS logic. Feeding 5V into GPIO 21 or GPIO 22 can damage the ESP32 silicon. Ensure the MPU6050 module is supplied from the **3.3V rail** of the ESP32 board.
- **Pull-Up Resistors:** Standard GY-521 / MPU6050 breakout modules feature integrated 4.7 kΩ pull-up resistors on SDA and SCL connected to the module's VCC line. No additional external resistors are required.

### 4.2 I2C Address Configuration
- **AD0 connected to GND (or left unconnected):** I2C Address = `0x68` (Default).
- **AD0 connected to 3.3V:** I2C Address = `0x69` (Alternate).
- The firmware automatically probes both `0x68` and `0x69` at initialization, providing plug-and-play compatibility regardless of AD0 configuration.

### 4.3 Power Delivery Options
1. **USB-UART Development Mode:** Direct connection via Micro-USB / Type-C cable to PC (5V regulated to 3.3V on-board by AMS1117 / ME6211).
2. **Li-Ion / LiPo Battery Mode (Portable Tag):** Single 3.7V 500–1200 mAh LiPo battery wired through a TP4056 charging module with protection circuit connected to the ESP32 5V/VIN or a dedicated 3.3V Low-Dropout (LDO) regulator pin.

---

## 5. Hardware Diagnostics Procedure
1. Verify all jumper connections with a digital multimeter (continuity test).
2. Connect ESP32 to computer via USB data cable.
3. Flash the I2C diagnostic scanner:
   ```bash
   arduino-cli compile --fqbn esp32:esp32:esp32 tests/i2c_scan
   arduino-cli upload -p <YOUR_COM_PORT> --fqbn esp32:esp32:esp32 tests/i2c_scan
   ```
4. Open Serial Monitor at **115200 baud**.
5. Confirm output indicates: `Device found at address 0x68 <-- MPU6050 (expected!)`.
