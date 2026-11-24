/**
 * TraceTag — Motion Detector
 * Analyzes accelerometer & gyroscope vectors to detect physical movement.
 */

#pragma once
#include "mpu6050_handler.h"

class MotionDetector {
public:
  MotionDetector();

  // Initialize threshold and baseline tracking
  void init(float threshold = 2.0f);

  // Process a new sensor sample
  void update(const SensorData& data);

  // Status checks
  bool isMotionDetected() const;
  float getMagnitude() const { return _currentMagnitude; }
  float getDeviation() const { return _currentDeviation; }
  float getGyroMagnitude() const { return _currentGyroMagnitude; }

  // Arm/disarm detection (e.g. only actively trigger during FIND mode)
  void setArmed(bool armed);
  bool isArmed() const { return _armed; }

  // Clear detected state after handling
  void resetMotionFlag();

private:
  float _threshold;            // Acceleration deviation threshold (m/s^2)
  float _baseline;             // Baseline magnitude (approximates ~9.81 m/s^2)
  float _currentMagnitude;     // Latest acceleration vector magnitude
  float _currentDeviation;     // Absolute deviation from baseline
  float _currentGyroMagnitude; // Latest gyro angular rate magnitude
  bool  _motionDetected;
  bool  _armed;
  int   _consecutiveTriggers;  // Debounce counter to reject transient noise

  static const int TRIGGER_CONFIRM_COUNT = 2; // Required consecutive triggers
};
