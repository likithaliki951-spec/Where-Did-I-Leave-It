import 'dart:async';
import 'dart:convert';

import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:permission_handler/permission_handler.dart';


void main() {

  WidgetsFlutterBinding.ensureInitialized();

  runApp(const MyApp());

}



class MyApp extends StatelessWidget {

  const MyApp({super.key});


  @override
  Widget build(BuildContext context) {

    return MaterialApp(

      debugShowCheckedModeBanner: false,

      title: "TraceTag",

      theme: ThemeData(

        colorScheme: ColorScheme.fromSeed(
          seedColor: Colors.blue,
        ),

        useMaterial3: true,

      ),

      home: const BluetoothPage(),

    );

  }

}





class BluetoothPage extends StatefulWidget {

  const BluetoothPage({super.key});


  @override
  State<BluetoothPage> createState() =>
      _BluetoothPageState();

}




class _BluetoothPageState
    extends State<BluetoothPage> {


  final Guid serviceUuid = Guid(
    "12345678-1234-1234-1234-1234567890AB",
  );


  final Guid characteristicUuid = Guid(
    "ABCDEFAB-1234-5678-1234-ABCDEF123456",
  );



  List<ScanResult> devices = [];



  BluetoothDevice? connectedDevice;


  BluetoothCharacteristic? characteristic;



  StreamSubscription? scanSubscription;


  StreamSubscription? notifySubscription;




  bool scanning = false;



  String connectionStatus =
      "Not Connected";



  String objectStatus =
      "Waiting";





  @override
  void initState() {

    super.initState();

    requestPermission();

  }






  Future<void> requestPermission() async {


    await Permission.bluetoothScan.request();


    await Permission.bluetoothConnect.request();


    await Permission.location.request();


  }







  Future<void> scanESP32() async {


    if(scanning) return;



    setState(() {

      scanning = true;

      devices.clear();

    });




    await scanSubscription?.cancel();




    scanSubscription =
        FlutterBluePlus.scanResults.listen(
      (results) {


        for(final result in results) {


          String name =
              result.device.platformName;



          if(name.isEmpty) {

            name =
                result.advertisementData.advName;

          }



          debugPrint(
            "DEVICE: $name ${result.device.remoteId}",
          );



          if(
            name.contains("WhereDidILeaveIt") ||
            result.advertisementData.serviceUuids
                .contains(serviceUuid)
          ) {



            bool exists =
                devices.any(
                  (d) =>
                  d.device.remoteId ==
                  result.device.remoteId,
                );



            if(!exists) {


              setState(() {

                devices.add(result);

              });


            }


          }


        }


      },

    );




    await FlutterBluePlus.startScan(

      timeout:
          const Duration(seconds:10),

    );



    await Future.delayed(

      const Duration(seconds:10),

    );



    await FlutterBluePlus.stopScan();



    setState(() {

      scanning = false;

    });


  }






  Future<void> connectESP32(
      BluetoothDevice device) async {


    try {


      await FlutterBluePlus.stopScan();



      setState(() {

        connectionStatus =
            "Connecting...";

      });



      await device.connect(

        timeout:
            const Duration(seconds:15),

      );



      connectedDevice = device;



      setState(() {

        connectionStatus =
            "Connected";

      });



      List<BluetoothService> services =
          await device.discoverServices();



      for(BluetoothService service in services) {


        if(service.uuid == serviceUuid) {


          for(BluetoothCharacteristic c
          in service.characteristics){
                    if(c.uuid == characteristicUuid) {


          characteristic = c;



          if(c.properties.notify) {


            await c.setNotifyValue(true);



            notifySubscription =
                c.lastValueStream.listen(
              (value) {


                if(value.isEmpty) {

                  return;

                }



                String message =
                    utf8.decode(value);



                debugPrint(
                    "ESP32: $message"
                );



                if(!mounted) return;



                setState(() {


                  if(message ==
                      "OBJECT_FOUND") {


                    objectStatus =
                        "Object Found 🔍";


                  }

                  else if(message ==
                      "READY") {


                    objectStatus =
                        "Device Ready";


                  }

                  else {


                    objectStatus =
                        message;


                  }


                });


              },

            );


          }


        }


      }


    }


  }





  ScaffoldMessenger.of(context)
      .showSnackBar(

    const SnackBar(

      content:
          Text(
            "Connected Successfully",
          ),

    ),

  );


}


catch(e) {


  debugPrint(
      "Connection Error: $e"
  );


  setState(() {


    connectionStatus =
        "Connection Failed";


  });


}

}
Future sendCommand(String command) async {


  if(characteristic == null) {

    return;

  }



  try {


    await characteristic!.write(

      utf8.encode(command),

      withoutResponse: false,

    );



    debugPrint(
      "Command Sent: $command",
    );


  }


  catch(e) {


    debugPrint(
      "Write Error: $e",
    );


  }


}





Future disconnectESP32() async {


  await notifySubscription?.cancel();


  await connectedDevice?.disconnect();



  connectedDevice = null;


  characteristic = null;



  setState(() {


    connectionStatus =
        "Disconnected";


    objectStatus =
        "Waiting";


  });


}





@override
void dispose() {


  scanSubscription?.cancel();


  notifySubscription?.cancel();


  super.dispose();


}
@override
Widget build(BuildContext context) {

  return Scaffold(

    appBar: AppBar(

      title:
          const Text(
            "TraceTag",
          ),

      centerTitle: true,

    ),



    body: Padding(

      padding:
          const EdgeInsets.all(16),


      child: Column(

        children: [



          Image.asset(

            "assets/logo.png",

            height: 90,

          ),



          const SizedBox(height: 10),



          const Text(

            "TraceTag",

            style: TextStyle(

              fontSize: 28,

              fontWeight: FontWeight.bold,

            ),

          ),



          const SizedBox(height: 20),





          Card(

            child: ListTile(

              leading:
                  const Icon(
                    Icons.bluetooth,
                    size: 35,
                  ),


              title:
                  const Text(
                    "Connection",
                  ),


              subtitle:
                  Text(
                    connectionStatus,
                  ),


            ),

          ),





          Card(

            child: ListTile(

              leading:
                  const Icon(
                    Icons.location_on,
                    size: 35,
                  ),


              title:
                  const Text(
                    "Object Status",
                  ),


              subtitle:
                  Text(
                    objectStatus,
                  ),


            ),

          ),





          const SizedBox(height: 20),





          ElevatedButton.icon(

            onPressed:
                scanning
                ? null
                : scanESP32,


            icon:
                const Icon(
                  Icons.search,
                ),


            label:
                Text(

                  scanning

                  ? "Scanning..."

                  : "Scan ESP32",

                ),


          ),





          Expanded(

            child:
                ListView.builder(


              itemCount:
                  devices.length,


              itemBuilder:
                  (context,index){



                final device =
                    devices[index].device;



                return Card(

                  child: ListTile(


                    leading:
                        const Icon(
                          Icons.memory,
                        ),



                    title:
                        Text(

                          device.platformName
                              .isEmpty

                          ? "WhereDidILeaveIt"

                          : device.platformName,

                        ),



                    subtitle:
                        Text(
                          device.remoteId.str,
                        ),



                    trailing:
                    ElevatedButton(


                      onPressed: () {


                        connectESP32(device);


                      },


                      child:
                          const Text(
                            "Connect",
                          ),


                    ),


                  ),


                );


              },


            ),

          ),





          Row(

            mainAxisAlignment:
                MainAxisAlignment.spaceEvenly,


            children: [


              ElevatedButton(

                onPressed:

                connectedDevice == null

                ? null

                : () {


                  sendCommand(
                    "FIND_OBJECT",
                  );


                },


                child:
                    const Text(
                      "Find Object",
                    ),


              ),





              ElevatedButton(

                onPressed:

                connectedDevice == null

                ? null

                : () {


                  sendCommand(
                    "STOP_FIND",
                  );


                },


                child:
                    const Text(
                      "Stop",
                    ),


              ),


            ],


          ),





          const SizedBox(height: 10),





          ElevatedButton(

            onPressed:

            connectedDevice == null

            ? null

            : disconnectESP32,


            child:
                const Text(
                  "Disconnect",
                ),


          ),



        ],


      ),

    ),


  );

}


}
            
            