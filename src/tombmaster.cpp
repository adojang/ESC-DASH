/*
  --------------------------------------------------------------------------
  Escape Room Template
  Adriaan van Wijk
  22 May 2023

  This code controls 4 relays which will arm and disarm electromagnetic locks.

  Copyright [2023] [Proxonics (Pty) Ltd]

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
  --------------------------------------------------------------------------
*/

#define NAME "thumbreader"
#define MACAD 0xC0 // Refer to Table in Conventions


/* Kernal*/
#include <Arduino.h>
#include <config.h>
#include <encode.h>

/* ESP-DASH */
#include <ArduinoJson.h>
#include <AsyncTimer.h>

/* Wifi and mDNS */
#include <WiFi.h>
#include <esp_wifi.h>
#include <ESPmDNS.h>

/* ESP-NOW */
#include <esp_now.h>

/* Elegant OTA */
#include <AsyncElegantOTA.h>




// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x00}; // Address of Master Server
uint8_t setMACAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, MACAD};


/* ESP Async Timer */
AsyncTimer asynctimer;

/* WiFi Credentials */
const char* ssid = WIFI_SSID; // SSID
const char* password = WIFI_PASS; // Password

/* ESP-NOW Structures */






 dataPacket sData; // data to send
 dataPacket rData; // data to recieve

/* Setup */
AsyncWebServer server(80);
esp_now_peer_info_t peerInfo;
String success;

// bool opendoor1 = false;
// bool opendoor2 = false;
// bool opendoor3 = false;
// bool opendoor4 = false;

//Fast Flash to show SENT Data Succesfully
void sendDataLED(){
  // If it works... it works...
  digitalWrite(2,HIGH);
  asynctimer.setTimeout([]() {digitalWrite(2,LOW);},  200);
  asynctimer.setTimeout([]() {digitalWrite(2,HIGH);},  400);
  asynctimer.setTimeout([]() {digitalWrite(2,LOW);},  600);
}



void triggerDoor(int pin, int timeout){
  digitalWrite(pin, LOW);
  Serial.println("Door Opened");
  asynctimer.setTimeout([pin]() {
      digitalWrite(pin, HIGH);
      Serial.println("Door Closed");
    }, 5000);
  
  
  // asynctimer.setInterval([]() {esp_now_send(broadcastAddress, (uint8_t *) &sData, sizeof(sData));},  5000);

}



/* ESP-NOW Callback Functions*/

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&rData, incomingData, sizeof(rData));

  Serial.println("Data Recieved...");
  
  
  // Forward Data from Sensors to Master Server
  if (rData.origin != masterserver){
        esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &rData, sizeof(rData));
  if (result == ESP_OK) {sendDataLED();}}


  // if((rData.sensor == train_keypad) && (rData.data == 1)){
  //   Serial.println("Train Door Triggered :)");
  //   Serial.print("Origin: ");
  //   Serial.println(rData.origin);
    
  //    triggerDoor(door1, doortime);
  // }








  // Add your code here to do something with the data recieved

}



 

void startwifi(){

  // Set device as a Wi-Fi Station
  WiFi.softAP(NAME, "pinecones", 0, 1, 4);
  WiFi.mode(WIFI_AP_STA);
  esp_wifi_set_mac(WIFI_IF_STA, &setMACAddress[0]);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.printf("WiFi Failed!\n");
      return;
  }
  else{
    Serial.println("WIFI CONNECTED!");
  }

   /* MDNS */
  if (!MDNS.begin(NAME)) {
        Serial.println("Error setting up MDNS responder!");
        while(1) {
            delay(1000);
        }
    }
  Serial.println("mDNS responder started");
  Serial.printf("*** PROGRAM START ***\n\n");
  
  AsyncElegantOTA.begin(&server, "admin", "admin1234");

  server.begin();
  MDNS.addService("http", "tcp", 80);

}

void startespnow(){
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  //Register Callback Functions
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

}

void setup() {
  Serial.begin(115200);
  startwifi();
  startespnow();

  pinMode(5, OUTPUT);
  pinMode(18, OUTPUT);
  pinMode(19, OUTPUT);
  pinMode(21, OUTPUT);
  digitalWrite(5, HIGH);
  delay(250);
  digitalWrite(18, HIGH);
  delay(250);
  digitalWrite(19, HIGH);
  delay(250);
  digitalWrite(21, HIGH);


  }




void loop() {

  //This line is sort of required. It automatically sends the data every 5 seconds. Don't know why. But hey there it is.



  asynctimer.handle();
}
