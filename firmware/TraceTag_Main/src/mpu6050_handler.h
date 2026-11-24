/**
 * TraceTag — MPU6050 6-Axis Motion Sensor Driver
 * Direct I2C implementation without external third-party dependencies.
 *
 * Hardware connections:
 *   ESP32 GPIO 21 -> SDA
 *   ESP32 GPIO 22 -> SCL
 *   3.3V          -> VCC
 *   GND           -> GND
 */

#pragma once
#include <Arduino.h>
#include <Wire.h>

// Sensor reading structure (SI units)
struct SensorData {
  float accel_x;    // Acceleration X in m/s^2
  float accel_y;    // Acceleration Y in m/s^2
  float accel_z;    // Acceleration Z in m/s^2
  float gyro_x;     // Angular velocity X in rad/s
  float gyro_y;     // Angular velocity Y in rad/s
  float gyro_z;     // Angular velocity Z in rad/s
  float temperature;// Internal die temperature in deg C
};

class MPU6050Handler {
public:
  MPU6050Handler(uint8_t address = 0x68);

  // Initialize I2C and configure MPU6050 registers
  bool begin(int sdaPin = 21, int sclPin = 22, uint32_t frequency = 100000);

  // Check if sensor is connected and responding on I2C
  bool isConnected();

  // Read latest 6-axis acceleration, gyro and temperature
  SensorData read();

  // Diagnostics
  bool isInitialized() const { return _initialized; }
  uint8_t getAddress() const { return _address; }

private:
  uint8_t _address;
  bool _initialized;
  int _sda;
  int _scl;

  bool writeRegister(uint8_t reg, uint8_t value);
  bool readRegisters(uint8_t reg, uint8_t *buffer, size_t count);
};
