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
#include <esp_task_wdt.h> // watchdog for doorlock mag recovery if it get stuck


#define NAME "sennet"
#define setMACAddress m_tomb_sennet
#define WDT_TIMEOUT 10 // 10 seconds

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
int pin13 = 0;
int pin32 = 0;
int pin33 = 0;
int pin34 = 0;
int pin35 = 0;


/* Functions */

void openDrawer()
{
  digitalWrite(13,LOW);
  delay(1000);
  digitalWrite(13,HIGH);
  // sData.origin = tomb_tangrum;
  // sData.sensor = tomb_tangrum;
  // sData.data = 1; // puzzle complete

  // esp_err_t result = esp_now_send(m_tombmaster, (uint8_t *) &sData, sizeof(sData));
  // if (result == ESP_OK) { Serial.println("Sent with success");}
  // else {Serial.println("Error sending the data");}

}


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // WebSerial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");
  }

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) 
{
  memcpy(&rData, incomingData, sizeof(rData));

  if(rData.origin == masterserver && rData.sensor == tomb_sennet && rData.data == 1){
    openDrawer();

  }

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
  sData.origin = tomb_sennet;
  sData.sensor = status_alive;
  esp_err_t result = esp_now_send(m_masterserver, (uint8_t *) &sData, sizeof(sData));
  esp_task_wdt_reset(); //restarts out after 10 seconds of not sending.
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

  //HIGH outputs
  pinMode(22,OUTPUT);
  pinMode(23,OUTPUT);
  pinMode(25,OUTPUT);
  pinMode(26,OUTPUT);
  pinMode(13,OUTPUT); // Relay Trigger Pin
  digitalWrite(13,HIGH);
  digitalWrite(22,HIGH);
  digitalWrite(23,HIGH);
  digitalWrite(25,HIGH);
  digitalWrite(26,HIGH);

  

  #pragma endregion gpio

  Core.startup(setMACAddress, NAME, server);
  startespnow();
  registermac(m_masterserver);
  registermac(m_tombmaster);

  Serial.println("Configuring WDT...");
  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch


  sData.origin = tomb_tangrum;
  sData.sensor = tomb_tangrum;

  pinMode(2,OUTPUT);
  digitalWrite(2,HIGH);

  asynctimer.setInterval([]() {statusUpdate();},  1000);
}



unsigned long ttimer = millis();
bool oneshotEnable = true;

// unsigned long ktimer = millis();
// int kcounter = 0;
void loop() {

  // if (millis() - ktimer > random(5000, 20001)){
  //   if (kcounter < 100)
  //   {
  //     ktimer = millis();
  //     kcounter += 1;
  //     WebSerial.printf("Number of Times Triggered: %d\n", kcounter);
  //     openDrawer();
  //   }
  //   if (kcounter > 100){
  //     WebSerial.printf("PASS - TEST SUCCESFFULL\n");
  //   }
  // }


if (totalc == 4 && millis() - ttimer > 2000 && oneshotEnable == true){
  ttimer = millis();
  Serial.printf("Puzzle Complete! 4/4\n");
  WebSerial.printf("Puzzle Complete! 4/4\n");
  openDrawer();
  oneshotEnable = false;

  //I need to use a oneshot flag here to prevent it from triggering 10000000 in a row. Poor relay :(
}

if(totalc < 4){
  oneshotEnable = true; // This prevents tomb from triggering multiple times. Oneshots only.
  //To trigger repeatedly, a piece has to be removed and replaced. Must read at least 12 or less for totalc :)
}

//Readpin has a small delay which regulates this loop.
totalc = 0;
readPin(17);
readPin(18);
readPin(19);
readPin(21);
WebSerial.printf("Puzzle Pieces: %d/4\n", totalc);
Serial.printf("Puzzle Pieces: %d/4\n", totalc);


  


  asynctimer.handle();
}