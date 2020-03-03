/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

int scanTime = 5; //In seconds
BLEScan* pBLEScan;

String Names[10];
int Device_RSSI[10];

uint8_t scan_position=0;
uint8_t scan_position_devices=0;

static BLEAdvertisedDevice* myDevice;

//variables para conexion 
static boolean doConnect= false;
static boolean connected =false;
static boolean doScan = false; 
// cuando llegan datos seriales para localizar un dispositivo 
static boolean searching_flag=false;
uint8_t found_device=0;
// Para conectarse con los beacons 
String incomming_String;
class MyClientCallback : public BLEClientCallbacks{
  // funcion para conectarse 
  void onConnect(BLEClient* pclient){
    
  }
  //funcion para desconectarse 
  void onDisconnect(BLEClient* pclient){
    connected = false;
    Serial.println("disconected");
  }
};

bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");

    connected = true;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      //Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
      if(advertisedDevice.haveName()== true){

        Names[scan_position]= advertisedDevice.getName().c_str();
        Device_RSSI[scan_position]=advertisedDevice.getRSSI();

        if(incomming_String!=""){
        if(incomming_String==Names[scan_position].c_str()){
          Serial.println("Encontre a zaboo");
          BLEDevice::getScan()->stop();
          myDevice = new BLEAdvertisedDevice(advertisedDevice);
          doConnect=true;
          doScan=true;
          // le sumamos 1 para que sea 1 si encuentra el dispositivo
          found_device +=1;     
         incomming_String="";
        }
        }

        scan_position ++; 
      }   
    }
};



void setup() {
  
  Serial.begin(115200);
  Serial.println("Scanning...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value

    // Limpiar los dos Array 
  for (int i=0; i< (sizeof(Device_RSSI)/sizeof(Device_RSSI[0]));i++){
    Device_RSSI[i]=0;
  }
   for (int i=0; i< (sizeof(Names)/sizeof(Names[0]));i++){
    Names[i]="";
  }

//   for (int i=0; i< (sizeof(NamesForCon)/sizeof(NamesForCon[0]));i++){
//    NamesForCon[i]="";
//  }
//   for (int i=0; i< (sizeof(myDeviceForCon)/sizeof(myDeviceForCon[0]));i++){
//    myDeviceForCon[i]=0;
//  }
}

void loop() {
  // put your main code here, to run repeatedly:
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  PrintJSON();
// Si Alguien está buscando beacons
 if(searching_flag==true){
      if(found_device==0){
        Serial.println("No se encontro el Dispositivo");
        incomming_String="";
      }
      searching_flag=false;
  }
  
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
  scan_position=0;
  scan_position_devices=0;
  // Limpiar los dos Array 
  for (int i=0; i< (sizeof(Device_RSSI)/sizeof(Device_RSSI[0]));i++){
    Device_RSSI[i]=0;
  }
   for (int i=0; i< (sizeof(Names)/sizeof(Names[0]));i++){
    Names[i]="";
  }

// Cuando llegue un mensaje del broer queriendose conectar a algun BEACON 
while(Serial.available()){
  delay(3);
  char c = Serial.read();
  incomming_String += c;
}
  incomming_String.trim();
  if(incomming_String.length() >0){
    //Verificar si tengo guardado el nombre del dispositivo en el array
    searching_flag=true;
    Serial.println(incomming_String);
  }

  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
    found_device=0;
  }
 
  delay(100);
  Serial.println(ESP.getFreeHeap());


}

void PrintJSON(){
  Serial.print("{\"devices\":[");
  for (int i=0; i< (sizeof(Names)/sizeof(Names[0]));i++){
  if(Names[i]==""){
    Serial.println("]}");
    break;
  }
  else{
    Serial.print("{\"name\":");
    Serial.print("\"");
    Serial.print(Names[i]);
    Serial.print("\",");
    Serial.print("\"rssi\":");
    Serial.print(Device_RSSI[i]);
    
    if(Names[i+1]==""){
      Serial.print("}");
    }
    else{
      Serial.print("},");
    }
  }
  }
  
}
