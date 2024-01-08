/*
--------------------------------------------------------------------------
                          Tygervallei Escape Room Project
--------------------------------------------------------------------------
  Author: Adriaan van Wijk
  Date: 8 January 2024

  This code is part of a multi-node project involving Escape Rooms in Tygervallei,
  South Africa.

  Copyright (c) 2023 Proxonics (Pty) Ltd

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at:

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
  --------------------------------------------------------------------------
*/

#include <EscCore.h>

#define NAME "trainmaster"
#define setMACAddress m_trainmaster

#pragma region mac
// Control
uint8_t m_masterserver[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x00};
uint8_t m_trainmaster[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x01};
uint8_t m_tombmaster[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x02};
uint8_t m_atticmaster[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x03};

uint8_t m_attic_light[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x09};

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
ESPDash dashboard(&server, false);
esp_now_peer_info_t peerInfo;
EscCore Core;

dataPacket sData; // data to send
dataPacket rData; // data to recieve

/* Configuration and Setup */
bool checkrfidstuck = false;
bool RFID1_reset = false;
bool RFID2_reset = false;
bool RFID3_reset = false;
bool RFID4_reset = false;

unsigned long emergencyTrigger = 0;
bool RFID1_complete = false;
bool RFID2_complete = false;
bool RFID3_complete = false;
bool RFID4_complete = false;
bool rfiddoorlock = false;
unsigned long ttime = millis();
unsigned long time250 = millis();
int keypadtrigger = 0;

unsigned short attic_status_timer = 0;
unsigned short clock_status_timer = 0;
unsigned short bike_status_timer = 0;
unsigned short RFID1_status_timer = 0;
unsigned short RFID2_status_timer = 0;
unsigned short RFID3_status_timer = 0;
unsigned short RFID4_status_timer = 0;
unsigned short thumbreader_status_timer = 0;
unsigned short tomb_status_timer = 0;
unsigned short tomb_chalice_timer = 0;
unsigned short tomb_tangrum_timer = 0;
unsigned short tomb_sennet_timer = 0;
unsigned short attic_bikebackup_timer = 0;

const int defaultDoorTimeout = 2000;


/* triggerDoor */
/**
 * Opens the train room door and sends a message to the keypads controller to update the lights.
 *
 * @param pin The pin number to which the door is connected.
 */
void triggerDoor(int pin)
{
  sData.origin = masterserver;
  sData.sensor = masterserver;
  sData.data = 1; // Door Open
  esp_err_t result = esp_now_send(m_train_keypad, (uint8_t *)&sData, sizeof(sData));

  digitalWrite(pin, HIGH);
  WebSerial.println("Door Opened");

  asynctimer.setTimeout([pin]()
                        {
                          sData.origin = masterserver;
                          sData.sensor = masterserver;
                          sData.data = 0; // Door closed
                          esp_err_t result = esp_now_send(m_train_keypad, (uint8_t *)&sData, sizeof(sData));
                          digitalWrite(pin, LOW);
                          WebSerial.println("Door Closed");

                          // delay(50);
                        },
                        defaultDoorTimeout);
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  // WebSerial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  memcpy(&rData, incomingData, sizeof(rData));

  if (rData.origin == train_keypad && rData.sensor == train_keypad)
  {
    keypadtrigger = rData.data;
  }

  if (rData.origin == masterserver && rData.data == 1){
    //Open DOOR
    triggerDoor(5);
  }



}

void startespnow()
{ // Remeber to register mac addresses before sending data;
  if (esp_now_init() != ESP_OK)
  {
    WebSerial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
}

void registermac(uint8_t address[])
{
  memcpy(peerInfo.peer_addr, address, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    WebSerial.println("Failed to add peer");
    return;
  }
}

void statusUpdate(){
  sData.origin = trainmaster;
  sData.sensor = status_alive;
  esp_err_t result = esp_now_send(m_masterserver, (uint8_t *) &sData, sizeof(sData));
}

void setup()
{
  Serial.begin(115200); // Not used, kept for redundancy.

  Core.startup(setMACAddress, NAME, server);
  startespnow();
  registermac(m_masterserver);
  registermac(m_train_thumb);
  registermac(m_train_keypad);


#pragma region gpio
  // Signaling LED
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);

  pinMode(25, OUTPUT);    // Emergency Buttons
  digitalWrite(25, HIGH); // 25 is trigger pin HIGH. Source.
  pinMode(34, INPUT);     // Pulldown input with 110nf cap and a resistor.

  pinMode(23, OUTPUT);
  digitalWrite(23, LOW); // 23 is GND makes a PULLDOWN resistor config.

  // Enable relay
  pinMode(5, OUTPUT);
  digitalWrite(5, LOW);

#pragma endregion gpio

  emergencyTrigger = millis();

  asynctimer.setInterval([]() {statusUpdate();},  1000);


}

unsigned long rtime = millis();

void loop()
{


  /* REMOVE this AFTER HARDWARE MOD */
  //Currently, I've chosen to keep the emergency button linked to software because of the complex interconnected nature of the signalling LED lights on the keypad.


  if ((analogRead(34) > 3500) && ((millis() - emergencyTrigger) >= 1000))
  {
    WebSerial.println(analogRead(34)); //
    emergencyTrigger = millis();
    WebSerial.println("Door Triggered");
    triggerDoor(5);
    // delay(50); // I'm gonna be removing thehse.
  }

  /* REMOVE AFTER HARDWARE MOD */

  if (keypadtrigger == 1) // Unlock the Door Steady State
  {
    WebSerial.println("Keypad Unlocked");
    keypadtrigger = 0;
    digitalWrite(5, HIGH);
  }

  if (keypadtrigger == 2) // Lock the Door Steady State
  {
    WebSerial.println("Keypad Locked");
    keypadtrigger = 0;
    digitalWrite(5, LOW);
  }

  asynctimer.handle();
}