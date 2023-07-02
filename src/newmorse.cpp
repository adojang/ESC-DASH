/*
  --------------------------------------------------------------------------
  Escape Room Template
  Adriaan van Wijk
  22 May 2023

  This code is part of a multi-node project which involes Esc Rooms in Tygervallei,
  South Africa.

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

#include <EscCore.h>

#define NAME "morse"
#define setMACAddress m_attic_morse

#pragma region mac
// Control
uint8_t m_masterserver[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x00};
uint8_t m_trainmaster[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x01};
uint8_t m_tombmaster[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x02};
uint8_t m_atticmaster[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x03};

// Attic
uint8_t m_attic_humanchain[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA0};
uint8_t m_attic_bike[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA1};
uint8_t m_clock[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA2};
uint8_t m_attic_overrideButton[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA3};
uint8_t m_RFID1[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA4};
uint8_t m_RFID2[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA5};
uint8_t m_RFID3[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA6};
uint8_t m_RFID4[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA7};
uint8_t m_attic_clock2[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA8};
uint8_t m_attic_morse[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA9};


// Tomb
uint8_t m_tomb_sennet[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xB0};
uint8_t m_tomb_chalice[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xB1};
uint8_t m_tomb_ringReader[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xB2};
uint8_t m_tomb_tangrum[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xB3};
uint8_t m_tomb_maindoorOverride[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xB4};
uint8_t m_tomb_slidedoorOverride[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xB5};

// Train
uint8_t m_train_keypad[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xC0};
uint8_t m_train_thumb[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xC1};
uint8_t m_train_overrideButton[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xC2};
#pragma endregion mac

AsyncTimer asynctimer(35);
AsyncWebServer server(80);
ESPDash dashboard(&server,false);
esp_now_peer_info_t peerInfo;
EscCore Core;

dataPacket sData; // data to send
dataPacket rData; // data to recieve

/* Configuration and Setup */

unsigned long ttime = millis();
// Morse Variables

int globalwait = 0;
int normalDelay = 650;

int longDelay = normalDelay * 2;
int morsebootflag = 0;
int morseperiod = 0;
unsigned long morsetimer = 0;
unsigned short IDx; // ID for morse loop.
int resetmorseflag = 0;

/* Functions */

void shortlight() {
  ledcWrite(0, 4000);
  digitalWrite(2,HIGH);
  asynctimer.setTimeout([]() {
    digitalWrite(2,LOW);
    ledcWrite(0, 0);
  }, normalDelay);
}

void longlight(){ // turn light on, then wait for delay to turn off again.
  ledcWrite(0, 4000);
  digitalWrite(2,HIGH);
  asynctimer.setTimeout([]() {
    ledcWrite(0, 0);
    digitalWrite(2,LOW);
  }, longDelay);
}

void breifpause(){
  globalwait += longDelay;
}

void shortpause(){
  globalwait += normalDelay;
}

void endloop(){

morseperiod = (millis()-morsetimer) + (2*longDelay);
morsebootflag = 1;
Serial.println("Morse Period:");
Serial.println(morseperiod);
}

void dot(){
  Serial.println("Dot");
  asynctimer.setTimeout([]() {shortlight(); }, globalwait);
    globalwait += normalDelay*2;
    Serial.printf("New Wait Time: %d\n", globalwait);
    }

void dash(){
  Serial.println("Dash");
  asynctimer.setTimeout([]() {longlight(); }, globalwait);
    globalwait += longDelay*2;
    Serial.printf("New Wait Time: %d\n", globalwait);
    }

void morseloop(){
morsetimer = millis();
globalwait = 0;

dot();
dot();
dot();
dash();
dash();
breifpause();
breifpause();

dot();
dot();
dot();
dot();
dot(); // delay slightly longer.

shortpause();
breifpause();
breifpause();

dot();
dash();
dash();
dash();
dash();
breifpause();
breifpause();

if (morsebootflag == 0){
  asynctimer.setTimeout([]() {endloop();}, globalwait); // wait for 6200 before executing
}

Serial.println("Morse End");
}



void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {WebSerial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) 
{
  memcpy(&rData, incomingData, sizeof(rData));
  
}


void startespnow(){ // Remeber to register mac addresses before sending data;
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
}

void registermac(uint8_t address[]){
  memcpy(peerInfo.peer_addr, address, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
}


void setup() {
  Serial.begin(115200);
  pinMode(2,OUTPUT);
  ledcSetup(0, 100, 12);
  ledcAttachPin(27, 0); 
  ledcWrite(0, 0);
  digitalWrite(2,LOW);
  morseloop();

  //Due to Shoddy Wifi Connection, this lamp will be completely offline.


  // Core.startup(setMACAddress, NAME, server);
  // startespnow();
  // registermac(m_masterserver);



}



void loop() {

 if(morsebootflag == 1)
  {
    ttime = millis();
    Serial.println("Immediate Trigger");
    morseloop();
    delay(100);
    IDx = asynctimer.setInterval([]() {
      Serial.println("Execute IDx");
      Serial.printf("Time since last loop: %d\n", millis()-ttime);
      ttime = millis();
      morseloop();
    }, morseperiod);
    morsebootflag = 2;
  }

  asynctimer.handle();
}