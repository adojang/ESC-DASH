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

#define NAME "beetle"
#define MACAD 0x04 // Refer to Table Below

/* Data Naming Convention for Mac Addresses
*  0x00 - masterserver
 * 0x01 - humanchain
 * 0x02 - bikelight
 * 0x03 - clockmotor
 * 0x04 - beetle
 * 0x05 - chalicedoor
 * 0x06 - ringreader
 * 0x07 - tangrumtomb
 * 0x08 - thumbreaderdoor
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

/* SET MAC ADDRESS */
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

/*Pin Variables*/
int Onboard_LED = 2;  //Onboard blue LED pin 
int ReedSwitchPin_1 = 27; //Reed switch output 1
int ReedSwitchPin_2 = 26; //Reed switch output 2
int ReedSwitchPin_3 = 25; //Reed switch output 3
int ReedSwitchPin_4 = 33; //Reed switch output 4
int MagneticLockRelayPin = 16;  //Maglock relay activation pin 

/*Other Global Variables*/
int PuzzleSolved = 0;
int PuzzlesolvedPrev = 0;
int MaglockActivationTrigger = 0;
int MaglockTimerStart = 0;
int CurrentMillis1 = 0;
int OverrideTrigger = 0;
int ledState = LOW;
unsigned long PreviousMillis = 0;
const long BlinkDelayInterval = 30;
unsigned long CurrentMillis2 = 0;

/* Setup */
AsyncWebServer server(80);
esp_now_peer_info_t peerInfo;

/*Function that is called to override magnetic lock from Master*/
void Override(){
  OverrideTrigger=1;
}

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");

}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&rData, incomingData, sizeof(rData));
  Serial.println("Override Data Recieved...");

  //Incoming Data is copied to rData. Do something with it here or in the main loop.
  //Incoming Data Goes Here
  if(rData.trigger==1){
  Override();
  }


}

void startespnow(){
    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
      return;
    }

    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(OnDataSent);
    
    // Register peer
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    
    // Add peer        
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
      Serial.println("Failed to add peer");
      return;
    }
    // Register for a callback function that will be called when data is received
    esp_now_register_recv_cb(OnDataRecv);
}

void startup(){
  /* Connect WiFi */
  WiFi.mode(WIFI_STA);
  esp_wifi_set_mac(WIFI_IF_STA, &setMACAddress[0]);
  
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
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
  
  AsyncElegantOTA.begin(&server, "admin", "admin1234");

  server.begin();
  MDNS.addService("http", "tcp", 80);

}



void Blink(){ //Function that blinks the blue LED onboard the ESP32
if (CurrentMillis2 - PreviousMillis >= BlinkDelayInterval) {
    // save the last time you blinked the LED
    PreviousMillis = CurrentMillis2;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(Onboard_LED, ledState);
  }
}

int IsPuzzleSolved(){ //Function that reads all the reed switch inputs and determines if puzzle is solved or not
  if(digitalRead(ReedSwitchPin_1)==1){ Serial.println("Reed1: ON");
    if(digitalRead(ReedSwitchPin_2)==1){Serial.println("Reed2: ON");
      if(digitalRead(ReedSwitchPin_3)==1){Serial.println("Reed3: ON");
        if(digitalRead(ReedSwitchPin_4)==1){Serial.println("Reed4: ON");         
          //Blink();         
          return 1;      
        }else{return 0;}
      }else{return 0;}
    }else{return 0;}
  }else{return 0;}  
}

void setup() {
  sData.trigger = 0;
  Serial.begin(115200);
  startup(); // Startup for Wifi, mDNS, and OTA
  startespnow(); // Startup for ESP-NOW
  pinMode(Onboard_LED,OUTPUT); //Onboard LED pin configuration
  pinMode(ReedSwitchPin_1, INPUT_PULLDOWN);  //Use Pull-Down resistor configuration with GPIO pin as digital input
  pinMode(ReedSwitchPin_2, INPUT_PULLDOWN);  //Use Pull-Down resistor configuration with GPIO pin as digital input
  pinMode(ReedSwitchPin_3, INPUT_PULLDOWN);  //Use Pull-Down resistor configuration with GPIO pin as digital input
  pinMode(ReedSwitchPin_4, INPUT_PULLDOWN);  //Use Pull-Down resistor configuration with GPIO pin as digital input

  //Begin Sending Data to Remote ESP's every 250ms
   asynctimer.setInterval([]() { esp_now_send(broadcastAddress, (uint8_t *) &sData, sizeof(sData));},  250);

}


void loop() {
  //Insert Code Here
  CurrentMillis2 = millis();
  PuzzlesolvedPrev=PuzzleSolved;
  PuzzleSolved=IsPuzzleSolved();
  if(OverrideTrigger==1){
  PuzzleSolved = rData.trigger;
  }
  sData.trigger=PuzzleSolved;
  if(PuzzlesolvedPrev==0 && PuzzleSolved==1){
    MaglockTimerStart=millis();
    MaglockActivationTrigger=1;
  }
  if(MaglockActivationTrigger==1){
    CurrentMillis1=millis();
    if((CurrentMillis1>=MaglockTimerStart)&&(CurrentMillis1<=(MaglockTimerStart+900))){
      digitalWrite(MagneticLockRelayPin, LOW);
      Blink();
    }
    else{
      digitalWrite(MagneticLockRelayPin, HIGH);
      MaglockActivationTrigger=0;
    }
  }
  //You need to edit OnDataRecv to handle incoming overrides.

  asynctimer.handle();
}

