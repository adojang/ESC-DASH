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

#define NAME "atticmaster"
#define MACAD 0x03 // Refer to Table in Conventions


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
int emergencyFlag = 0;
int emergencyTrigger = 0;
const unsigned long emergencyButtonTimeout = 5000;

bool RFID1 = false;
bool RFID2 = false;
bool RFID3 = false;
bool RFID4 = false;
bool DOORTOUCH = false;

int readingcounter = 0;

void getTouch(){

  int reading = analogRead(35);

  if(reading > 3900){
    // Serial.printf("ABOVE THRESHOLD\n\n\n\n");
    readingcounter +=25;
  }


    if (readingcounter > 100) {
      readingcounter = 0;
       Serial.printf("ABOVE THRESHOLD\n\n\n\n");
      DOORTOUCH = true;
    }
    readingcounter -= 5;
    if (readingcounter < 0) readingcounter = 0;
}


//This Sends Data to the MasterServer to tell it its still connected
void statusAlive(){
  sData.origin = atticmaster;
  sData.sensor = atticmaster;
  sData.data = 0;
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &rData, sizeof(rData));
}


void triggerDoor(int pin, int timeout){
  digitalWrite(pin, LOW);
  Serial.println("Door Opened");
  delay(15);
  asynctimer.setTimeout([pin]() {
      digitalWrite(pin, HIGH);
      Serial.println("Door Closed");
    }, 5000);
  
  
  // asynctimer.setInterval([]() {esp_now_send(broadcastAddress, (uint8_t *) &sData, sizeof(sData));},  5000);

}





void IRAM_ATTR emergency(){
  emergencyFlag = 1;

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
  }


    if (rData.origin == masterserver && rData.sensor == attic_humanchain && rData.data == 1){
      triggerDoor(5, 2000);
      triggerDoor(18, 2000);
      triggerDoor(19, 2000);
      triggerDoor(21, 2000);
    }

       if (rData.origin == attic_RFID2 && rData.sensor == attic_RFID2 && rData.data == 1){
      triggerDoor(5, 2000);
    }


    



     if(rData.origin == attic_bike && rData.sensor == attic_bike)
    {
    //Send Data to LED to light up appropriately
      //4000 / 100 = 40
      ledcWrite(1, ((40)*rData.data));
      Serial.println(rData.data);

    }









  // Add your code here to do something with the data recieved

}


void startwifi(){

  // Set device as a Wi-Fi Station
  WiFi.softAP(NAME, "pinecones", 0, 1, 4);
  WiFi.mode(WIFI_AP_STA);
  esp_wifi_set_mac(WIFI_IF_STA, &setMACAddress[0]);
  unsigned long wifitimeout = millis();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
    if ((millis() - wifitimeout) > 10000) ESP.restart();
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

int normalDelay = 650;
int longDelay = normalDelay * 2;
void shortlight(){

  ledcWrite(0, 4000);
  digitalWrite(2,HIGH);
  delay(normalDelay);
  ledcWrite(0, 0);
  digitalWrite(2,LOW);
  delay(normalDelay);
}
void longlight(){
  ledcWrite(0, 4000);
  digitalWrite(2,HIGH);
  delay(longDelay);
  digitalWrite(2,LOW);
  ledcWrite(0, 0);
  delay(normalDelay);

}

void morseloop(){
//Spell ESC 

//E - 3 delay units
shortlight();
delay(normalDelay);
//S - 7 delay units
shortlight();
shortlight();
shortlight();
delay(normalDelay);
//C - 12 delay units
longlight();
shortlight();
longlight();
shortlight();
delay(longDelay);
delay(longDelay);
Serial.println("Morse");
//total 24 delay units
}


void setup() {
  Serial.begin(115200);
  startwifi();
  startespnow();

  pinMode(22, OUTPUT); // emergency Button GND
  digitalWrite(22, HIGH);
  pinMode(23, INPUT_PULLDOWN); // emergency Button
  pinMode(2, OUTPUT);
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

 attachInterrupt(17, emergency, HIGH);

  // LIGHT TEST

  ledcSetup(0, 100, 12); // Lightbulb
  ledcSetup(1, 100, 12); // Bike LED
  ledcAttachPin(27, 0); // Lightbulb
  ledcAttachPin(26, 1);
  ledcWrite(0, 0);
  ledcWrite(1, 0);

  analogReadResolution(12);
  //GND Pin for PWM Controllers
  pinMode(33, OUTPUT);
  digitalWrite(33, LOW);
  pinMode(25, OUTPUT);
  digitalWrite(25, LOW);

  //HumanTouch Pins
  pinMode(35, INPUT);
  pinMode(15, OUTPUT);
  digitalWrite(15,HIGH);


    //  asynctimer.setInterval([]() {
    //   statusAlive();
    //   ;},  1000);


    int timeout = normalDelay*24;
    // Serial.println(timeout);
    // Serial.println("MorseLooooop Trigger0");
     
    //  asynctimer.setInterval([]() {
    //   Serial.println("MorseLooooop Trigger");
    //   morseloop();
    // }, timeout);

    asynctimer.setInterval([]() {getTouch();},  25);
    asynctimer.setInterval([]() {Serial.println(readingcounter);},  1000);


     emergencyTrigger = millis();
  }




int timeout = millis();
void loop() {

    if (DOORTOUCH){
    //Open the marvelous door.
// ***************************************************************** EDIT THIS LATER TO INCLUDE THE BOOL FROM THE PUZZLES
  Serial.println("Human Chain Touch Detected!");
  sData.origin = attic_humanchain;
  sData.sensor = attic_humanchain;
  sData.data = 1;
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sData, sizeof(sData));

    DOORTOUCH = false;
  triggerDoor(5, 2000);
  delay(75);
  triggerDoor(18, 2000);
  delay(75);
  triggerDoor(19, 2000);
  delay(75);
  triggerDoor(21, 2000);
  delay(75);
  }


//Emergency Escape Button
if ((digitalRead(23)) && (millis() - emergencyTrigger) >= 5000)
  {
    emergencyTrigger = millis();
    emergencyFlag = 0;
  Serial.println("OH NO HERE WE GO");
  sData.origin = 0x03;
  sData.sensor = 0xA3;
  sData.data = 1;
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sData, sizeof(sData));
  
  triggerDoor(5, 2000);
  triggerDoor(18, 2000);
  triggerDoor(19, 2000);
  triggerDoor(21, 2000);

  if (result == ESP_OK) { 
    Serial.println("Sent with success");
    }
  else {
    Serial.println("Error sending the data");
  }
  delay(50);
}





  asynctimer.handle();
}