# Where Did I Leave It?

## Project Overview

Where Did I Leave It? is an IoT-based object tracking and location-recording system designed to help users remember where they last placed their personal belongings.

The project uses an ESP32 microcontroller and a motion sensor to detect object movement and identify when an object has been placed in a stationary position.

The system is designed to communicate with a mobile application through Bluetooth Low Energy (BLE). The mobile application is intended to record the object's last known location using the smartphone's GPS.

## Objectives

- Detect movement of personal belongings using a motion sensor.
- Identify when an object becomes stationary.
- Communicate object status through Bluetooth Low Energy.
- Record the last known location using a mobile application.
- Help users locate misplaced belongings.

## Hardware Requirements

- ESP32 Development Board
- MPU-compatible motion sensor module
- Breadboard
- Jumper wires
- USB data cable
- Android smartphone

## Technologies

- Arduino IDE
- Embedded C/C++
- Flutter
- Dart
- Bluetooth Low Energy (BLE)
- GPS and Location Services

## Working Principle

1. The motion sensor detects changes in the object's movement.
2. The ESP32 processes the sensor readings.
3. When the object remains stationary, the system identifies a placement event.
4. The ESP32 communicates the event through BLE.
5. The planned mobile application records the event and the smartphone's location.
6. Users can refer to the last recorded location to find their belongings.

## Current Development Status

The hardware prototype has been tested for sensor communication, basic movement detection, stationary-state detection, and BLE initialization.

The Flutter mobile application and complete integration are under development.

## Future Enhancements

- Support for multiple tracked objects.
- Mobile application integration with BLE.
- GPS-based last-known-location history.
- Map-based location display.
- Improved movement detection and notification handling.

## Project Repository

This repository will contain the source code, documentation, and updates as development progresses.

**Project Status:** In Progress
