/**
 * TraceTag — Where Did I Leave It?
 * Smart Object Tracking & BLE Locator Firmware for ESP32
 *
 * Hardware:
 *   ESP32 Dev Module (WROOM-32)
 *   MPU6050 6-Axis Accelerometer & Gyroscope (I2C)
 *     - SDA: GPIO 21
 *     - SCL: GPIO 22
 *     - VCC: 3.3V
 *     - GND: GND
 *   Status / Alert LED: GPIO 2 (ESP32 Onboard LED)
 *   Optional Buzzer:    GPIO 23
 *
 * BLE Protocol Specifications (Extracted from Flutter App):
 *   - Device Name:         WhereDidILeaveIt
 *   - Service UUID:        12345678-1234-1234-1234-1234567890AB
 *   - Characteristic UUID: ABCDEFAB-1234-5678-1234-ABCDEF123456
 *   - Supported Commands:  FIND_OBJECT, STOP_FIND
 *   - Notification Msgs:   READY, OBJECT_FOUND
 */

#include <Arduino.h>
#include <Wire.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "src/mpu6050_handler.h"
#include "src/motion_detector.h"

// ---------------- Hardware Pin Definitions ----------------
#define PIN_I2C_SDA     21
#define PIN_I2C_SCL     22
#define PIN_LED_ALERT    2   // Built-in blue LED on ESP32 DevKit
#define PIN_BUZZER      23   // Optional active/passive buzzer

#define SERIAL_BAUD 115200

// ---------------- BLE Protocol Constants ----------------
#define BLE_DEVICE_NAME        "WhereDidILeaveIt"
#define SERVICE_UUID           "12345678-1234-1234-1234-1234567890AB"
#define CHARACTERISTIC_UUID    "ABCDEFAB-1234-5678-1234-ABCDEF123456"

#define CMD_FIND_OBJECT        "FIND_OBJECT"
#define CMD_STOP_FIND          "STOP_FIND"

#define RESP_READY             "READY"
#define RESP_OBJECT_FOUND      "OBJECT_FOUND"

// ---------------- Application States ----------------
enum TagState {
  STATE_STANDBY,     // Connected or disconnected, sensor in passive mode
  STATE_FINDING,     // FIND_OBJECT active, alert pulsing, awaiting motion
  STATE_FOUND        // Motion triggered, OBJECT_FOUND notified
};

// Global instances
MPU6050Handler imu;
MotionDetector motionDetector;

BLEServer *pServer = nullptr;
BLECharacteristic *pCharacteristic = nullptr;

bool deviceConnected = false;
bool oldDeviceConnected = false;
TagState currentState = STATE_STANDBY;

// Timing & non-blocking control
unsigned long lastSensorPoll = 0;
unsigned long lastLedToggle = 0;
unsigned long lastHeartbeat = 0;
bool ledState = false;

// Forward declaration
void sendNotification(const char* message);

// ---------------- BLE Server Callbacks ----------------
class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    deviceConnected = true;
    Serial.println("[BLE] Client connected.");
  }

  void onDisconnect(BLEServer* pServer) override {
    deviceConnected = false;
    Serial.println("[BLE] Client disconnected. Restarting advertising...");
  }
};

// ---------------- BLE Characteristic Callbacks ----------------
class CharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pChar) override {
    String rxValue = pChar->getValue();

    if (rxValue.length() > 0) {
      // Trim potential trailing newlines/carriage returns
      rxValue.trim();
      Serial.print("[BLE RX] Command received: ");
      Serial.println(rxValue);

      if (rxValue.equals(CMD_FIND_OBJECT)) {
        currentState = STATE_FINDING;
        motionDetector.resetMotionFlag();
        motionDetector.setArmed(true);
        Serial.println("[TAG] Mode -> FINDING (Searching active, motion armed)");
      }
      else if (rxValue.equals(CMD_STOP_FIND)) {
        currentState = STATE_STANDBY;
        motionDetector.setArmed(false);
        motionDetector.resetMotionFlag();
        digitalWrite(PIN_LED_ALERT, LOW);
        digitalWrite(PIN_BUZZER, LOW);
        Serial.println("[TAG] Mode -> STANDBY (Search stopped)");
        sendNotification(RESP_READY);
      }
      else {
        Serial.print("[BLE RX] Unknown command: ");
        Serial.println(rxValue);
      }
    }
  }
};

// Helper: send UTF-8 BLE notification to connected Flutter app
void sendNotification(const char* message) {
  if (deviceConnected && pCharacteristic != nullptr) {
    pCharacteristic->setValue((uint8_t*)message, strlen(message));
    pCharacteristic->notify();
    Serial.print("[BLE TX] Notified: ");
    Serial.println(message);
  }
}

// ---------------- Setup ----------------
void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(500);

  Serial.println("\n==============================================");
  Serial.println("  TraceTag — Where Did I Leave It? Firmware");
  Serial.println("  ESP32 BLE Object Tracker & Motion Locator");
  Serial.println("==============================================\n");

  // Configure alert GPIOs
  pinMode(PIN_LED_ALERT, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_LED_ALERT, LOW);
  digitalWrite(PIN_BUZZER, LOW);

  // Initialize MPU6050 on GPIO 21 (SDA) and GPIO 22 (SCL)
  Serial.print("[I2C] Initializing MPU6050 (SDA=");
  Serial.print(PIN_I2C_SDA);
  Serial.print(", SCL=");
  Serial.print(PIN_I2C_SCL);
  Serial.println(")...");

  if (imu.begin(PIN_I2C_SDA, PIN_I2C_SCL, 100000)) {
    Serial.print("[I2C] MPU6050 detected at address 0x");
    Serial.println(imu.getAddress(), HEX);
    motionDetector.init(2.0f); // 2.0 m/s^2 deviation threshold
    Serial.println("[OK] Motion detection subsystem ready.");
  } else {
    Serial.println("[WARN] MPU6050 not detected. Check I2C wiring (SDA=21, SCL=22).");
    Serial.println("[WARN] BLE will continue to operate for testing.");
  }

  // ---------------- Initialize BLE Subsystem ----------------
  Serial.print("[BLE] Initializing device: ");
  Serial.println(BLE_DEVICE_NAME);

  BLEDevice::init(BLE_DEVICE_NAME);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  // Create primary GATT service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create GATT characteristic with Read, Write & Notify properties
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_NOTIFY
  );

  pCharacteristic->setCallbacks(new CharacteristicCallbacks());

  // Add BLE2902 descriptor for standard client characteristic notification
  pCharacteristic->addDescriptor(new BLE2902());

  // Initial characteristic value
  pCharacteristic->setValue(RESP_READY);

  // Start the GATT service
  pService->start();

  // Configure and start BLE Advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // Compatibility with iOS/Android
  pAdvertising->setMinPreferred(0x12);

  BLEDevice::startAdvertising();
  Serial.println("[BLE] Advertising started. Waiting for Flutter client...\n");
}

// ---------------- Main Loop ----------------
void loop() {
  unsigned long now = millis();

  // Handle connection state transitions
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = true;
    delay(100);
    // Notify Flutter client that device is READY upon connection
    sendNotification(RESP_READY);
  }

  if (!deviceConnected && oldDeviceConnected) {
    oldDeviceConnected = false;
    currentState = STATE_STANDBY;
    digitalWrite(PIN_LED_ALERT, LOW);
    digitalWrite(PIN_BUZZER, LOW);
    // Restart advertising when client disconnects
    delay(200);
    pServer->startAdvertising();
    Serial.println("[BLE] Re-enabled advertising.");
  }

  // Periodic sensor sampling (~20 Hz / 50ms)
  if (now - lastSensorPoll >= 50) {
    lastSensorPoll = now;

    if (imu.isInitialized()) {
      SensorData data = imu.read();
      motionDetector.update(data);

      // If in FINDING state and motion is detected:
      if (currentState == STATE_FINDING && motionDetector.isMotionDetected()) {
        currentState = STATE_FOUND;
        Serial.println("[TAG] *** MOTION DETECTED! Object has been moved/found. ***");
        sendNotification(RESP_OBJECT_FOUND);

        // Solid alert indicator for 1 second
        digitalWrite(PIN_LED_ALERT, HIGH);
        digitalWrite(PIN_BUZZER, HIGH);
      }
    } else {
      // Periodic background probe if MPU6050 was not detected at boot
      static unsigned long lastProbe = 0;
      if (now - lastProbe > 5000) {
        lastProbe = now;
        if (imu.begin(PIN_I2C_SDA, PIN_I2C_SCL, 100000)) {
          Serial.println("[I2C] MPU6050 hot-plug detected! Subsystem active.");
          motionDetector.init(2.0f);
        }
      }
    }
  }

  // Visual/audible feedback state machine
  switch (currentState) {
    case STATE_FINDING:
      // Pulse LED and buzzer at 4 Hz (every 125ms) while finding
      if (now - lastLedToggle >= 125) {
        lastLedToggle = now;
        ledState = !ledState;
        digitalWrite(PIN_LED_ALERT, ledState ? HIGH : LOW);
        digitalWrite(PIN_BUZZER, ledState ? HIGH : LOW);
      }
      break;

    case STATE_FOUND:
      // Double flash pattern indicating found status
      if (now - lastLedToggle >= 250) {
        lastLedToggle = now;
        ledState = !ledState;
        digitalWrite(PIN_LED_ALERT, ledState ? HIGH : LOW);
      }
      break;

    case STATE_STANDBY:
    default:
      // Standby: LED off
      digitalWrite(PIN_LED_ALERT, LOW);
      digitalWrite(PIN_BUZZER, LOW);
      break;
  }

  // Periodic heartbeat on serial (every 5 seconds)
  if (now - lastHeartbeat >= 5000) {
    lastHeartbeat = now;
    Serial.print("[STATUS] State: ");
    if (currentState == STATE_STANDBY) Serial.print("STANDBY");
    else if (currentState == STATE_FINDING) Serial.print("FINDING");
    else if (currentState == STATE_FOUND) Serial.print("FOUND");

    Serial.print(" | BLE: ");
    Serial.print(deviceConnected ? "CONNECTED" : "ADVERTISING");

    Serial.print(" | IMU: ");
    Serial.println(imu.isInitialized() ? "ONLINE" : "OFFLINE");
  }

  delay(5); // Small yield for RTOS scheduler
}
