/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            15

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      8
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
int delayval = 50; // delay for half a second

int scanTime = 5; //In seconds
BLEScan* pBLEScan;
uint8_t state=0;
String Names[10];

int Device_RSSI[10];
// index del arreglo spara nombres y RSSI 
uint8_t scan_position=0;


static BLEAdvertisedDevice* myDevice;
static BLEClient*  pClient;

//variables para conexion 
static boolean doConnect= false;
static boolean connected =false;
static boolean doScan = false; 
// cuando llegan datos seriales para localizar un dispositivo 
static boolean searching_flag=false;
uint8_t found_device=0;
// Para conectarse con los beacons 
String incomming_String;
String led_status="";


class MyClientCallback : public BLEClientCallbacks{
  // funcion para conectarse 
  void onConnect(BLEClient* pclient){
    
  }
  //funcion para desconectarse 
  void onDisconnect(BLEClient* pclient){
    connected = false;
     delete myDevice;
    delete pclient; //crash
    Serial.println("disconected");
  }
};

bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    //if(pClient!=nullptr){
    //  delete(pClient);
    //}
    pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");
    
    pClient->setClientCallbacks(new MyClientCallback(), true); // optional pa

    // Connect to the remove BLE Server.
    bool _connected = pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    if(!_connected){
      delete pClient;
      delete myDevice;
      return false;
    }
    Serial.println(" - Connected to server");

    //delay(4000);
    //pClient->disconnect();
    //Serial.println("desconectar");
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
    // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code

  pixels.begin(); // This initializes the NeoPixel library.
  
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
}

void loop() {

 if(state==0){
  Serial.println("S0");
    while(Serial.available()){
      delay(3);
      char c = Serial.read();
      incomming_String += c;
    }
    delay(10);
    Serial.println(incomming_String);
    if (strncmp (incomming_String.c_str(),"START",5) == 0){
      color_ring(0,0,0);
      state=1;
      incomming_String="";
    }
    else{
      incomming_String="";
    }
    
    color_ring(0,150, 0);
    
 }

 if(state==1){
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

    // Limpiar los dos Array 
    for (int i=0; i< (sizeof(Device_RSSI)/sizeof(Device_RSSI[0]));i++){
      Device_RSSI[i]=0;
    }
    for (int i=0; i< (sizeof(Names)/sizeof(Names[0]));i++){
      Names[i]="";
    }

    // Cuando llegue un mensaje del broker queriendose conectar a algun BEACON 
    while(Serial.available()){
      delay(3);
      char c = Serial.read();
      incomming_String += c;
    }
     incomming_String.trim();
    if(incomming_String.length() >0){
         if(incomming_String[0]!='Z'){
        led_status=incomming_String;
        incomming_String="";
      }
     else{
      searching_flag=true;
     }
      
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

    if(led_status!=""){
      Serial.println("Orden cambio de color");
      color_ring(150,0,150);
      Serial.println(led_status);
      led_status="";
    }
 }
    delay(100);
    //Serial.println(ESP.getFreeHeap());
 
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

void color_ring(int R, int G, int B){
    // For a set of NeoPixels the first NeoPixel is 0, second is 1, all the way up to the count of pixels minus one.

  for(int i=0;i<NUMPIXELS;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(R,G,B)); // Moderately bright green color.
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(delayval); // Delay for a period of time (in milliseconds).
  }
  
    for(int i=0;i<NUMPIXELS;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(0,0,0)); // Moderately bright green color.
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(delayval); // Delay for a period of time (in milliseconds).
  }
  return;
  
}
