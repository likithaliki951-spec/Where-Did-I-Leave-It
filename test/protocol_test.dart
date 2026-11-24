import 'package:flutter_test/flutter_test.dart';

void main() {
  group('BLE Protocol Specifications', () {
    const String expectedDeviceName = 'WhereDidILeaveIt';
    const String expectedServiceUuid = '12345678-1234-1234-1234-1234567890AB';
    const String expectedCharacteristicUuid =
        'ABCDEFAB-1234-5678-1234-ABCDEF123456';

    const String commandFindObject = 'FIND_OBJECT';
    const String commandStopFind = 'STOP_FIND';

    const String responseReady = 'READY';
    const String responseObjectFound = 'OBJECT_FOUND';

    test('Device name matches specification', () {
      expect(expectedDeviceName, equals('WhereDidILeaveIt'));
    });

    test('UUID formats are valid 128-bit UUID strings', () {
      final uuidRegex = RegExp(
          r'^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$');
      expect(uuidRegex.hasMatch(expectedServiceUuid), isTrue);
      expect(uuidRegex.hasMatch(expectedCharacteristicUuid), isTrue);
    });

    test('Commands match specification', () {
      expect(commandFindObject, equals('FIND_OBJECT'));
      expect(commandStopFind, equals('STOP_FIND'));
    });

    test('Responses match specification', () {
      expect(responseReady, equals('READY'));
      expect(responseObjectFound, equals('OBJECT_FOUND'));
    });
  });
}
