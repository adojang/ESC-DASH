/*
  --------------------------------------------------------------------------
  Escape Room Template
  Adriaan van Wijk
  22 May 2023

  Give a short explaination of what this code does and for what puzzle it is.

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

#define NAME "template"
#define MACAD 0xEE // Refer to Table in Conventions

/* Data Naming Convention for Mac Addresses

*  0x00 - masterserver


 * 0xA0 - humanchain
 * 0xA1 - bikelight
 * 0xA2 - clockmotor

 * 0xB0 - beetle
 * 0xB1 - chalicessensor
 * 0xB2 - ringreader
 * 0xB3 - tangrumtomb

 * 0xC0 - thumbreader
 * 0xC1 - Keypad 1
 * 0xC2 - Keypad 2

 * 0xD0 - relaycontrol

 * 0xEE - template

*/




/* Kernal*/
#include <Arduino.h>
#include <config.h>

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
typedef struct dataPacket {
int trigger = 0;
} dataPacket;

dataPacket sData; // data to send
dataPacket rData; // data to recieve

/* Setup */
AsyncWebServer server(80);
esp_now_peer_info_t peerInfo;


bool turnlighton = false;


/* Example Function on how to send data to another ESP that you can remove*/

void sendData()
{
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sData, sizeof(sData));
      if (result == ESP_OK) { Serial.println("Sent with success");}
      else {Serial.println("Error sending the data");}

}

/* ESP-NOW Callback Functions*/

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&rData, incomingData, sizeof(rData));
  Serial.println("Override Data Recieved...");

  if (rData.trigger == 1) {
  //You should never use delay in this function. It might cause the ESP-NOW to crash.
    turnlighton = true;
  }
  else
  {
    turnlighton = false;
  }
  
  // Add your code here to do something with the data recieved.
  //It's probably best to use a flag instead of calling it directly here. Not Sure

  //Demonstration Sending Data:

  sendData();


}
 

int lastState = LOW;
unsigned long lastChangeTime = 0;
const unsigned long debounceDelay = 10000;  // debounce time in microseconds

void getTouch(){



    if (touchRead(4) < 40) {
      turnlighton = true;
    }
    else {
      turnlighton = false;
    }
}

void signalGen(){

  digitalWrite(5, HIGH);  // Set the pin to HIGH
  asynctimer.setTimeout([]() {digitalWrite(5, LOW);},  1000);


}

void signalCheck(){
  int currentState = digitalRead(16);
  Serial.println(currentState);
  if (currentState != lastState) {
    // check if the state change isn't due to noise
    if ((micros() - lastChangeTime) > debounceDelay) {
      if (currentState == HIGH) {
        Serial.println("Signal detected!");
        turnlighton = true;
      } else {
        Serial.println("Signal lost!");
        turnlighton = false;
      }
    }
    lastState = currentState;
    lastChangeTime = micros();
  }

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

  //Make any Edits you need to add below this line ------------------------------

  pinMode(2, OUTPUT);
  digitalWrite(2,LOW);
  pinMode(16, INPUT);
  pinMode(5, OUTPUT);



  //Here are two asynchronus timers you can use to run functions.
  //See https://github.com/Aasim-A/AsyncTimer
  // For documentations

//    asynctimer.setInterval([]() {getTouch();},  100);
   asynctimer.setInterval([]() {signalCheck();},  500);
   asynctimer.setInterval([]() {signalGen();},    2000);
  // asynctimer.setTimeout([]() {Serial.println("Hello world!");}, 2000);
// "Hello world!" will be printed to the Serial once after 2 seconds
}





void loop() {



  if (turnlighton) {
    digitalWrite(2,HIGH);
  }
  else {
    digitalWrite(2,LOW);
  }






  //Required for the asynctimer to work.
  asynctimer.handle();
}