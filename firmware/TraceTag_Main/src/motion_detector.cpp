/**
 * TraceTag — Motion Detector Implementation
 * Uses adaptive baseline filtering and dual accel+gyro thresholds.
 */

#include "motion_detector.h"
#include <math.h>

MotionDetector::MotionDetector()
  : _threshold(2.0f),
    _baseline(9.81f),
    _currentMagnitude(9.81f),
    _currentDeviation(0.0f),
    _currentGyroMagnitude(0.0f),
    _motionDetected(false),
    _armed(true),
    _consecutiveTriggers(0) {}

void MotionDetector::init(float threshold) {
  _threshold = threshold;
  _baseline = 9.81f;
  _currentMagnitude = 9.81f;
  _currentDeviation = 0.0f;
  _currentGyroMagnitude = 0.0f;
  _motionDetected = false;
  _armed = true;
  _consecutiveTriggers = 0;
}

void MotionDetector::setArmed(bool armed) {
  _armed = armed;
  if (!armed) {
    _motionDetected = false;
    _consecutiveTriggers = 0;
  }
}

void MotionDetector::resetMotionFlag() {
  _motionDetected = false;
  _consecutiveTriggers = 0;
}

void MotionDetector::update(const SensorData& data) {
  // Compute linear acceleration magnitude: |a| = sqrt(ax^2 + ay^2 + az^2)
  _currentMagnitude = sqrtf(
    data.accel_x * data.accel_x +
    data.accel_y * data.accel_y +
    data.accel_z * data.accel_z
  );

  // Absolute deviation from baseline (normally ~1g / 9.81 m/s^2 at rest)
  _currentDeviation = fabsf(_currentMagnitude - _baseline);

  // Compute angular velocity magnitude: |w| = sqrt(gx^2 + gy^2 + gz^2)
  _currentGyroMagnitude = sqrtf(
    data.gyro_x * data.gyro_x +
    data.gyro_y * data.gyro_y +
    data.gyro_z * data.gyro_z
  );

  // Significant physical movement causes either acceleration deviation or rotation
  bool accelExceeded = (_currentDeviation > _threshold);
  bool gyroExceeded  = (_currentGyroMagnitude > 0.75f); // ~43 deg/s rotation
  bool activeMotion  = accelExceeded || gyroExceeded;

  if (_armed && activeMotion) {
    _consecutiveTriggers++;
    if (_consecutiveTriggers >= TRIGGER_CONFIRM_COUNT) {
      _motionDetected = true;
    }
  } else {
    if (_consecutiveTriggers > 0) {
      _consecutiveTriggers--;
    }

    // Slowly track resting baseline when sensor is quiet (drift compensation)
    if (_currentDeviation < 0.4f && _currentGyroMagnitude < 0.2f) {
      _baseline = 0.96f * _baseline + 0.04f * _currentMagnitude;
    }
  }
}

bool MotionDetector::isMotionDetected() const {
  return _motionDetected;
}
