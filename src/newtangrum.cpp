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

/* RFID */
#include <SPI.h>
#include <MFRC522.h>

#define NAME "tangrum"
#define setMACAddress m_tomb_tangrum

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
uint8_t reading = 0;
int totalc;
int pin17 = 0;
int pin18 = 0;
int pin19 = 0;
int pin21 = 0;
int pin22 = 0;
int pin23 = 0;
int pin25 = 0;
int pin26 = 0;
int pin27 = 0;
int pin32 = 0;
int pin33 = 0;
int pin34 = 0;
int pin35 = 0;


/* Functions */

void sendData()
{
  sData.origin = tomb_tangrum;
  sData.sensor = tomb_tangrum;
  sData.data = 1; // puzzle complete

  esp_err_t result = esp_now_send(m_tombmaster, (uint8_t *) &sData, sizeof(sData));
  if (result == ESP_OK) { Serial.println("Sent with success");}
  else {Serial.println("Error sending the data");}

}


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // WebSerial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");
  }

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

void statusUpdate(){
  sData.origin = tomb_tangrum;
  sData.sensor = status_alive;
  esp_err_t result = esp_now_send(m_masterserver, (uint8_t *) &sData, sizeof(sData));
}

void readPin(uint8_t pin)
{
  for (int i = 0; i < 10; i++)
  {
    reading = reading + digitalRead(pin);
    delay(10);
  }
  // Serial.println(reading);

    if (reading > 8)
    {
      totalc++;

    //   Serial.printf("Pin %d: 1 \n", pin);
    //   switch (pin)
    //   {
    //   case 17:
    //     pin17 = 1;
    //     totalc++;
    //     break;
    //   case 18:
    //     pin18 = 1;
    //     totalc++;
    //     break;
    //   case 19:
    //     pin19 = 1;
    //     totalc++;
    //     break;
    //   case 21:
    //     pin21 = 1;
    //     totalc++;
    //     break;
    //   case 22:
    //     pin22 = 1;
    //     totalc++;
    //     break;
    //   case 23:
    //     pin23 = 1;
    //     totalc++;
    //     break;
    //   case 25:
    //     pin25 = 1;
    //     totalc++;
    //     break;
    //   case 26:
    //     pin26 = 1;
    //     totalc++;
    //     break;
    //   case 27:
    //     pin27 = 1;
    //     totalc++;
    //     break;
    //   case 32:
    //     pin32 = 1;
    //     totalc++;
    //     break;
    //   case 33:
    //     pin33 = 1;
    //     totalc++;
    //     break;
    //   case 34:
    //     pin34 = 1;
    //     totalc++;
    //     break;
    //   case 35:
    //     pin35 = 1;
    //     totalc++;
    //     break;
    //   default:
    //     break;
    //   }
    // }
    // else
    // {
    //   Serial.printf("Pin %d: 0 \n", pin);
    //   switch (pin)
    //   {
    //   case 17:
    //     pin17 = 0;
    //     break;
    //   case 18:
    //     pin18 = 0;
    //     break;
    //   case 19:
    //     pin19 = 0;
    //     break;
    //   case 21:
    //     pin21 = 0;
    //     break;
    //   case 22:
    //     pin22 = 0;
    //     break;
    //   case 23:
    //     pin23 = 0;
    //     break;
    //   case 25:
    //     pin25 = 0;
    //     break;
    //   case 26:
    //     pin26 = 0;
    //     break;
    //   case 27:
    //     pin27 = 0;
    //     break;
    //   case 32:
    //     pin32 = 0;
    //     break;
    //   case 33:
    //     pin33 = 0;
    //     break;
    //   case 34:
    //     pin34 = 0;
    //     break;
    //   case 35:
    //     pin35 = 0;
    //     break;
    //   default:
    //     break;
    //   }
    // }
    
}
reading = 0;
}


void setup() {
  Serial.begin(115200);

  #pragma region gpio
  pinMode(17,INPUT_PULLDOWN);
  pinMode(18,INPUT_PULLDOWN);
  pinMode(19,INPUT_PULLDOWN);
  pinMode(21,INPUT_PULLDOWN);
  pinMode(22,INPUT_PULLDOWN);
  pinMode(23,INPUT_PULLDOWN);
  pinMode(25,INPUT_PULLDOWN);
  pinMode(26,INPUT_PULLDOWN);
  pinMode(27,INPUT_PULLDOWN);
  pinMode(32,INPUT_PULLDOWN);
  pinMode(33,INPUT_PULLDOWN);
  pinMode(34,INPUT);
  pinMode(35,INPUT);
  

  #pragma endregion gpio

  Core.startup(setMACAddress, NAME, server);
  startespnow();
  registermac(m_masterserver);
  registermac(m_tombmaster);

  sData.origin = tomb_tangrum;
  sData.sensor = tomb_tangrum;

  SPI.begin();        // Init SPI bus



  asynctimer.setInterval([]() {statusUpdate();},  1000);
}



unsigned long ttimer = millis();
bool oneshotEnable = true;
void loop() {


if (totalc == 13 && millis() - ttimer > 2000 && oneshotEnable == true){
  ttimer = millis();
  Serial.printf("Puzzle Complete! 13/13\n");
  WebSerial.printf("Puzzle Complete! 13/13\n");
  sendData();
  oneshotEnable = false;

  //I need to use a oneshot flag here to prevent it from triggering 10000000 in a row. Poor relay :(
}

if(totalc < 13){
  oneshotEnable = true; // This prevents tomb from triggering multiple times. Oneshots only.
  //To trigger repeatedly, a piece has to be removed and replaced. Must read at least 12 or less for totalc :)
}

//Readpin has a small delay which regulates this loop.
totalc = 0;
readPin(17);
readPin(18);
readPin(19);
readPin(21);
readPin(22);
readPin(23);
readPin(25);
readPin(26);
readPin(27);
readPin(32);
readPin(33);
readPin(34);
readPin(35);
WebSerial.printf("Puzzle Pieces: %d/13\n", totalc);
Serial.printf("Puzzle Pieces: %d/13\n", totalc);


  


  asynctimer.handle();
}