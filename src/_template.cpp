#define NAME "king"
#define MACAD 0x00 // Refer to Table in Conventions


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

AsyncTimer asynctimer(15);
AsyncWebServer server(80);
// ESPDash dashboard(&server,false);
esp_now_peer_info_t peerInfo;
EscCore Core;

dataPacket sData; // data to send
dataPacket rData; // data to recieve

uint8_t setMACAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, MACAD}; // Address of This Device
uint8_t broadcastAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x00}; // Address of Master Server


// void buttonTimeout(Card* cardptr, int timeout = 3000){
//   cardptr->update(1);
//   asynctimer.setTimeout([cardptr]() {
//     cardptr->update(0);
//     dashboard.sendUpdates();
//   }, timeout);
//   dashboard.sendUpdates();
// }


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&rData, incomingData, sizeof(rData));
  //
}


void startespnow(){
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

  //Starts Wifi, mDNS, OTA, and WebSerial
  Core.startup(setMACAddress, NAME, server);
  startespnow();
  registermac(broadcastAddress);




}

unsigned long ttime = millis();

void loop() {

if (millis() - ttime > 2000){
  // Serial.println("Hi...");
  ttime = millis();
}


asynctimer.handle();
}