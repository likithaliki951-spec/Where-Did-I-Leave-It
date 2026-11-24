#!/usr/bin/env python3
"""
TraceTag Automated Test Suite
Validates:
1. BLE Protocol Specification matching between Flutter App and ESP32 Firmware
2. Firmware Code Integrity & Pin Mapping (SDA=21, SCL=22)
3. Asset Path & File Integrity (logo.png resolution)
4. State Machine & Notification Protocol Logic
5. Automated Compilation Check for ESP32 Firmware using arduino-cli
"""

import os
import re
import subprocess
import sys
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent

class TestTraceTagProtocol(unittest.TestCase):
    """Verifies BLE Protocol invariants between Flutter and ESP32."""

    EXPECTED_DEVICE_NAME = "WhereDidILeaveIt"
    EXPECTED_SERVICE_UUID = "12345678-1234-1234-1234-1234567890AB"
    EXPECTED_CHAR_UUID = "ABCDEFAB-1234-5678-1234-ABCDEF123456"
    EXPECTED_CMD_FIND = "FIND_OBJECT"
    EXPECTED_CMD_STOP = "STOP_FIND"
    EXPECTED_RESP_READY = "READY"
    EXPECTED_RESP_FOUND = "OBJECT_FOUND"

    def test_flutter_main_dart_protocol(self):
        main_dart = REPO_ROOT / "lib" / "main.dart"
        self.assertTrue(main_dart.exists(), "lib/main.dart must exist")
        content = main_dart.read_text(encoding="utf-8")

        self.assertIn(self.EXPECTED_SERVICE_UUID, content, "Service UUID mismatch in Flutter")
        self.assertIn(self.EXPECTED_CHAR_UUID, content, "Characteristic UUID mismatch in Flutter")
        self.assertIn(self.EXPECTED_DEVICE_NAME, content, "Device name mismatch in Flutter")
        self.assertIn(self.EXPECTED_CMD_FIND, content, "FIND_OBJECT command missing in Flutter")
        self.assertIn(self.EXPECTED_CMD_STOP, content, "STOP_FIND command missing in Flutter")
        self.assertIn(self.EXPECTED_RESP_READY, content, "READY response handling missing in Flutter")
        self.assertIn(self.EXPECTED_RESP_FOUND, content, "OBJECT_FOUND handling missing in Flutter")

    def test_esp32_firmware_protocol(self):
        firmware_ino = REPO_ROOT / "firmware" / "TraceTag_Main" / "TraceTag_Main.ino"
        self.assertTrue(firmware_ino.exists(), "TraceTag_Main.ino must exist")
        content = firmware_ino.read_text(encoding="utf-8")

        self.assertIn(self.EXPECTED_DEVICE_NAME, content, "Device name missing in firmware")
        self.assertIn(self.EXPECTED_SERVICE_UUID, content, "Service UUID missing in firmware")
        self.assertIn(self.EXPECTED_CHAR_UUID, content, "Characteristic UUID missing in firmware")
        self.assertIn(self.EXPECTED_CMD_FIND, content, "FIND_OBJECT command missing in firmware")
        self.assertIn(self.EXPECTED_CMD_STOP, content, "STOP_FIND command missing in firmware")
        self.assertIn(self.EXPECTED_RESP_READY, content, "READY response missing in firmware")
        self.assertIn(self.EXPECTED_RESP_FOUND, content, "OBJECT_FOUND response missing in firmware")

    def test_hardware_pin_mapping(self):
        firmware_ino = REPO_ROOT / "firmware" / "TraceTag_Main" / "TraceTag_Main.ino"
        content = firmware_ino.read_text(encoding="utf-8")

        # SDA must be 21, SCL must be 22
        sda_match = re.search(r'#define\s+PIN_I2C_SDA\s+21', content)
        scl_match = re.search(r'#define\s+PIN_I2C_SCL\s+22', content)
        self.assertIsNotNone(sda_match, "SDA must be pinned to GPIO 21")
        self.assertIsNotNone(scl_match, "SCL must be pinned to GPIO 22")


class TestAssetAndDependencyIntegrity(unittest.TestCase):
    """Verifies assets and pubspec integrity."""

    def test_pubspec_structure(self):
        pubspec = REPO_ROOT / "pubspec.yaml"
        self.assertTrue(pubspec.exists(), "pubspec.yaml must exist")
        content = pubspec.read_text(encoding="utf-8")

        self.assertIn("assets/logo.png", content, "assets/logo.png must be declared in pubspec")
        self.assertIn("flutter_blue_plus", content, "flutter_blue_plus dependency missing")
        self.assertIn("permission_handler", content, "permission_handler dependency missing")

    def test_asset_files_exist(self):
        asset_logo = REPO_ROOT / "assets" / "logo.png"
        self.assertTrue(asset_logo.exists(), "assets/logo.png must exist in root assets directory")
        self.assertGreater(asset_logo.stat().st_size, 1000, "assets/logo.png must not be empty")

        # Verify original recovered files were NOT deleted
        original_assests = REPO_ROOT / "assests" / "logo.png"
        self.assertTrue(original_assests.exists(), "Original recovered assests/logo.png must be preserved")

    def test_android_manifest_permissions(self):
        manifest = REPO_ROOT / "android" / "app" / "src" / "main" / "AndroidManifest.xml"
        self.assertTrue(manifest.exists(), "AndroidManifest.xml must exist")
        content = manifest.read_text(encoding="utf-8")

        self.assertIn("android.permission.BLUETOOTH_SCAN", content)
        self.assertIn("android.permission.BLUETOOTH_CONNECT", content)
        self.assertIn("android.permission.ACCESS_FINE_LOCATION", content)


class TestFirmwareCompilation(unittest.TestCase):
    """Verifies that the ESP32 firmware compiles with arduino-cli without errors."""

    ARDUINO_CLI = Path(r"C:\Users\likit\AppData\Local\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe")

    def test_arduino_cli_compilation(self):
        if not self.ARDUINO_CLI.exists():
            self.skipTest(f"arduino-cli not found at {self.ARDUINO_CLI}")

        sketch_dir = REPO_ROOT / "firmware" / "TraceTag_Main"
        cmd = [
            str(self.ARDUINO_CLI),
            "compile",
            "--fqbn",
            "esp32:esp32:esp32",
            str(sketch_dir)
        ]
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
        self.assertEqual(
            result.returncode,
            0,
            f"Firmware compilation failed with error:\nSTDOUT: {result.stdout}\nSTDERR: {result.stderr}"
        )

    def test_i2c_scanner_compilation(self):
        if not self.ARDUINO_CLI.exists():
            self.skipTest(f"arduino-cli not found at {self.ARDUINO_CLI}")

        sketch_dir = REPO_ROOT / "tests" / "i2c_scan"
        cmd = [
            str(self.ARDUINO_CLI),
            "compile",
            "--fqbn",
            "esp32:esp32:esp32",
            str(sketch_dir)
        ]
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
        self.assertEqual(
            result.returncode,
            0,
            f"I2C scanner compilation failed with error:\nSTDOUT: {result.stdout}\nSTDERR: {result.stderr}"
        )


if __name__ == "__main__":
    unittest.main(verbosity=2)
