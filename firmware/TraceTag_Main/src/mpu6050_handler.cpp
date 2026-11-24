/**
 * TraceTag — MPU6050 Driver Implementation
 * Communicates directly with MPU6050 over I2C bus via Wire.h.
 */

#include "mpu6050_handler.h"

// MPU6050 Register Map
#define MPU6050_SMPLRT_DIV   0x19
#define MPU6050_CONFIG       0x1A
#define MPU6050_GYRO_CONFIG  0x1B
#define MPU6050_ACCEL_CONFIG 0x1C
#define MPU6050_ACCEL_XOUT_H 0x3B
#define MPU6050_PWR_MGMT_1   0x6B
#define MPU6050_WHO_AM_I     0x75

MPU6050Handler::MPU6050Handler(uint8_t address)
  : _address(address), _initialized(false), _sda(21), _scl(22) {}

bool MPU6050Handler::begin(int sdaPin, int sclPin, uint32_t frequency) {
  _sda = sdaPin;
  _scl = sclPin;

  Wire.begin(_sda, _scl, frequency);
  delay(50);

  // Probe address 0x68 first, then try 0x69
  uint8_t whoami = 0;
  if (!readRegisters(MPU6050_WHO_AM_I, &whoami, 1) || (whoami != 0x68 && whoami != 0x72)) {
    _address = 0x69;
    if (!readRegisters(MPU6050_WHO_AM_I, &whoami, 1) || (whoami != 0x68 && whoami != 0x72)) {
      _initialized = false;
      return false;
    }
  }

  // Wake device up by clearing sleep bit (PWR_MGMT_1 = 0x00)
  if (!writeRegister(MPU6050_PWR_MGMT_1, 0x00)) {
    _initialized = false;
    return false;
  }
  delay(15);

  // Set sample rate divider (1 kHz / (1 + 4) = 200 Hz sample rate)
  writeRegister(MPU6050_SMPLRT_DIV, 0x04);

  // Set DLPF (Digital Low Pass Filter) to ~44 Hz accel, 42 Hz gyro
  writeRegister(MPU6050_CONFIG, 0x03);

  // Set Gyroscope full scale range: +/- 500 deg/s (sensitivity 65.5 LSB/(deg/s))
  writeRegister(MPU6050_GYRO_CONFIG, 0x08);

  // Set Accelerometer full scale range: +/- 8g (sensitivity 4096 LSB/g)
  writeRegister(MPU6050_ACCEL_CONFIG, 0x10);

  _initialized = true;
  return true;
}

bool MPU6050Handler::isConnected() {
  uint8_t whoami = 0;
  if (!readRegisters(MPU6050_WHO_AM_I, &whoami, 1)) return false;
  return (whoami == 0x68 || whoami == 0x72);
}

bool MPU6050Handler::writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(_address);
  Wire.write(reg);
  Wire.write(value);
  return (Wire.endTransmission() == 0);
}

bool MPU6050Handler::readRegisters(uint8_t reg, uint8_t *buffer, size_t count) {
  Wire.beginTransmission(_address);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;

  size_t bytesRead = Wire.requestFrom((int)_address, (int)count);
  if (bytesRead != count) return false;

  for (size_t i = 0; i < count; i++) {
    buffer[i] = Wire.read();
  }
  return true;
}

SensorData MPU6050Handler::read() {
  SensorData data = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  if (!_initialized) return data;

  uint8_t raw[14];
  if (!readRegisters(MPU6050_ACCEL_XOUT_H, raw, 14)) {
    return data;
  }

  int16_t ax = (int16_t)((raw[0] << 8) | raw[1]);
  int16_t ay = (int16_t)((raw[2] << 8) | raw[3]);
  int16_t az = (int16_t)((raw[4] << 8) | raw[5]);
  int16_t tempRaw = (int16_t)((raw[6] << 8) | raw[7]);
  int16_t gx = (int16_t)((raw[8] << 8) | raw[9]);
  int16_t gy = (int16_t)((raw[10] << 8) | raw[11]);
  int16_t gz = (int16_t)((raw[12] << 8) | raw[13]);

  // Accelerometer conversion: +/- 8g range -> 4096 LSB/g, 1g = 9.80665 m/s^2
  const float ACCEL_SCALE = 9.80665f / 4096.0f;
  data.accel_x = (float)ax * ACCEL_SCALE;
  data.accel_y = (float)ay * ACCEL_SCALE;
  data.accel_z = (float)az * ACCEL_SCALE;

  // Temperature conversion: Temp (deg C) = (raw / 340.0) + 36.53
  data.temperature = ((float)tempRaw / 340.0f) + 36.53f;

  // Gyroscope conversion: +/- 500 deg/s range -> 65.5 LSB/(deg/s) -> convert to rad/s
  const float GYRO_SCALE = (3.14159265f / 180.0f) / 65.5f;
  data.gyro_x = (float)gx * GYRO_SCALE;
  data.gyro_y = (float)gy * GYRO_SCALE;
  data.gyro_z = (float)gz * GYRO_SCALE;

  return data;
}
