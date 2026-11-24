/**
 * TraceTag — I2C Scanner Diagnostic
 * 
 * Run this BEFORE the main firmware.
 * Scans all I2C addresses and prints found devices to Serial Monitor.
 * 
 * Expected output for MPU6050: Device found at address 0x68
 * 
 * Wiring:
 *   GPIO 21 → SDA
 *   GPIO 22 → SCL
 *   3.3V    → VCC
 *   GND     → GND
 */

#include <Wire.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }

  Serial.println("\n--- TraceTag I2C Scanner ---");
  Serial.println("Scanning I2C bus...\n");

  Wire.begin(21, 22);  // SDA=GPIO21, SCL=GPIO22
}

void loop() {
  int devicesFound = 0;

  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    uint8_t error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("Device found at address 0x");
      if (addr < 16) Serial.print("0");
      Serial.print(addr, HEX);

      // Identify common devices
      if (addr == 0x68 || addr == 0x69) {
        Serial.print("  <-- MPU6050 (expected!)");
      }
      Serial.println();
      devicesFound++;
    } else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (addr < 16) Serial.print("0");
      Serial.println(addr, HEX);
    }
  }

  if (devicesFound == 0) {
    Serial.println("No I2C devices found.");
    Serial.println("Check your wiring and 3.3V power supply.");
  } else {
    Serial.print("\nTotal devices found: ");
    Serial.println(devicesFound);
  }

  Serial.println("\nRescanning in 5 seconds...");
  Serial.println("----------------------------");
  delay(5000);
}
