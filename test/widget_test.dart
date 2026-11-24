import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:trace_tag/main.dart';

void main() {
  testWidgets('TraceTag UI smoke test - initial screen widgets',
      (WidgetTester tester) async {
    // Build our app and trigger a frame.
    await tester.pumpWidget(const MyApp());

    // Verify AppBar title is 'TraceTag'
    expect(find.text('TraceTag'), findsWidgets);

    // Verify initial connection status
    expect(find.text('Connection'), findsOneWidget);
    expect(find.text('Not Connected'), findsOneWidget);

    // Verify initial object status
    expect(find.text('Object Status'), findsOneWidget);
    expect(find.text('Waiting'), findsOneWidget);

    // Verify Scan button is present
    expect(find.text('Scan ESP32'), findsOneWidget);

    // Verify action buttons are present
    expect(find.text('Find Object'), findsOneWidget);
    expect(find.text('Stop'), findsOneWidget);
    expect(find.text('Disconnect'), findsOneWidget);

    // Verify icons are present
    expect(find.byIcon(Icons.bluetooth), findsOneWidget);
    expect(find.byIcon(Icons.location_on), findsOneWidget);
    expect(find.byIcon(Icons.search), findsOneWidget);
  });

  testWidgets('TraceTag action buttons disabled when disconnected',
      (WidgetTester tester) async {
    await tester.pumpWidget(const MyApp());

    // Find the buttons
    final findObjectBtn = tester.widget<ElevatedButton>(
      find.widgetWithText(ElevatedButton, 'Find Object'),
    );
    final stopBtn = tester.widget<ElevatedButton>(
      find.widgetWithText(ElevatedButton, 'Stop'),
    );
    final disconnectBtn = tester.widget<ElevatedButton>(
      find.widgetWithText(ElevatedButton, 'Disconnect'),
    );

    // Since connectedDevice is null initially, onPressed should be null (disabled)
    expect(findObjectBtn.onPressed, isNull);
    expect(stopBtn.onPressed, isNull);
    expect(disconnectBtn.onPressed, isNull);
  });
}
