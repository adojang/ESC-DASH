/*
  --------------------------------------------------------------------------
  Escape Room Master Server
  Adriaan van Wijk
  22 May 2023

  This code is for a server which listens for messages from the remote ESP's
  and enables override functions on the remote ESP's to override puzzles.

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

//https://docs.espdash.pro/cards/button/

#define NAME "trees"

/* Kernal*/
#include <Arduino.h>
#include <config.h>
#include "encode.h"
/* ESP-DASH */
#include <ESPDashPro.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTimer.h>

/* Wifi and mDNS */
#include <WiFi.h>
#include <esp_wifi.h>
#include <ESPmDNS.h>

/* ESP-NOW */
#include <esp_now.h>

/* Elegant OTA */
#include <AsyncElegantOTA.h>
#include <ESPmDNS.h>

/*************************WebSerial ****************************************/
#include <WebSerial.h>
/* Message callback of WebSerial */
void recvMsg(uint8_t *data, size_t len){
  WebSerial.println("Received Data...");
  String d = "";
  for(int i=0; i < len; i++){
    d += char(data[i]);
  }
  WebSerial.println(d);

  if (d == "ready"){
    WebSerial.println("You Are Ready for Action!");
  }
}

/*************************WebSerial ****************************************/

#define ADDRESSSLENGTH 11

/* SET MAC ADDRESS */
uint8_t setMACAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x00}; // Address of Master Server

/* ESP Async Timer */
AsyncTimer asynctimer(15);

int lightstat = 0;
void morseloop(){
  if (lightstat == 0){
    digitalWrite(2,HIGH);
    lightstat = 1;
  } else{
    digitalWrite(2,LOW);
    lightstat = 0;
  }
}

unsigned short IDx;

void setup() {
  Serial.begin(115200);
//LED 2 is the blue onboard light of the ESP32
  pinMode(2, OUTPUT);
  digitalWrite(2,LOW);
  lightstat = 0;
  // digitalWrite(2,HIGH);

IDx = asynctimer.setInterval([]() {
      Serial.printf(".");
      morseloop();
    }, 50);

}

unsigned long ttime = millis();

void loop() {

//CLEAN CLEAN CLEAN THIS LINE
if (millis() - ttime > 4000){
    digitalWrite(2,LOW);
    Serial.println();
    Serial.println("Cancel Test");
    asynctimer.cancel(IDx);
  ttime = millis();
}

  asynctimer.handle();
}