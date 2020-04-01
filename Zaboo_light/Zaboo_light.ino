#include <WiFi.h>
#include <PubSubClient.h>
#include "esp_system.h"

const char* ssid = "FOXO";
const char* password ="jugodenaranja07";
const char* mqttServer = "m16.cloudmqtt.com";
const int mqttPort = 11782;
const char* mqttUser = "mqtyljtw";
const char* mqttPassword ="O6eOiCQUH74k";
int mqttCon=0;


int UPCount=0;
int L1=14;

// Reinicio del Whatchdog
const int loopTimeCtl = 0;
hw_timer_t *timerW = NULL;

WiFiClient espClient;
PubSubClient client(espClient);

//digital outputs
int GL=18;
int BL=5; 
//Reinicio del whatchdog

//void IRAM_ATTR resetModule(){
 //   ets_printf("reboot\n");
  //  esp_restart();
//}


// Susbribing to a topic
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i< length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println(" ");

if(strncmp(topic,"Light1",length)==0){
if(strncmp((char*)payload,"ON",length)==0){
    Serial.println("L1ON");
    digitalWrite(L1,HIGH);
}

else if(strncmp((char*)payload,"OFF",length)==0){
    Serial.println("L1OF");
    digitalWrite(L1,LOW);
}

else if(strncmp((char*)payload,"SE",length)==0){
    digitalWrite(GL,HIGH);
}

else if(strncmp((char*)payload,"NOSE",length)==0){
    digitalWrite(GL,LOW);
}

else if(strncmp((char*)payload,"STATE",length)==0){
    client.publish("Light1","alive");
}


}

}
 
void setup() {
 
  Serial.begin(9600);
 WiFi.begin(ssid, password);

  pinMode(GL,OUTPUT);
  pinMode(BL,OUTPUT);
  pinMode(L1,OUTPUT);
 
  
  digitalWrite(L1,LOW);
  digitalWrite(BL,LOW);
  digitalWrite(GL,LOW);

 
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi..");
    digitalWrite(BL,HIGH);
  delay(500);
  }
 
  Serial.println("Connected to the WiFi network");
  
  digitalWrite(BL,LOW);
 
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
 
  while (!client.connected()&& mqttCon==0 ) {
    Serial.println("Connecting to MQTT");
 
    if (client.connect("Light1Client", mqttUser, mqttPassword )) {
 
      Serial.println("Connected to Broker");
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
      mqttCon=1;
 
    }
  }
   
    client.subscribe("Light1");

  //whatch dog rutine
  //timerW = timerBegin(0, 80, true); //timer 0, div 80
  //timerAttachInterrupt(timerW, &resetModule, true);
  //timerAlarmWrite(timerW, 10000000, false); //set time in us
  //timerAlarmEnable(timerW); //enable interrupt
    
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("Light1Client", mqttUser, mqttPassword )) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("Light1");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}


 
void loop() {
  //timerWrite(timerW, 0); //reset timer (feed watchdog)
  long loopTime = millis();
    if (!client.connected() && WiFi.status()==WL_CONNECTED) {
    reconnect();
  }
  
  client.loop();
  delay(10);

  if(WiFi.status()==WL_CONNECTED){
    delay(100);
    
  }
  else{
    WiFi.begin(ssid, password);
    int WLcount=0;
    while(WiFi.status() != WL_CONNECTED && WLcount <200){
      delay(100);
      Serial.printf(".");
      if(UPCount>=60){
        UPCount=0;
        Serial.printf("\n");
        
      }
      ++UPCount;
      ++WLcount;
    }
  }
//loopTime = millis() - loopTime;
//Serial.println(loopTime);
}
