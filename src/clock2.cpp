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

#define NAME "clock2"
#define MACAD 0xA8 // Refer to Table in Conventions


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


#include <Wire.h>
#include <VL53L0X.h>

VL53L0X sensor;

const int outPin = 15;
int covered = 0;

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x00}; // Address of Master Server
uint8_t setMACAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, MACAD};



/* ESP Async Timer */
AsyncTimer asynctimer(35);





void setup() {
  Serial.begin(115200);

  Wire.begin();
  sensor.init();
  sensor.setTimeout(500);

  pinMode(outPin, OUTPUT);


  sensor.startContinuous();
  }

void loop()
{
  // Serial.print(sensor.readRangeContinuousMillimeters());
  if (sensor.timeoutOccurred()) {
    Serial.printf("TIMEOUT Occured\n");
    //ERROR STATE.  NEED TO RESTART
   
    sensor.stopContinuous();
    Wire.endTransmission();
    delay(600);
    sensor.init();
    sensor.setTimeout(500);
    sensor.startContinuous();
    ESP.restart();
   }

  // Serial.println();

  if (sensor.readRangeContinuousMillimeters()<100) {
    Serial.println("Covered");
    covered = HIGH;
    digitalWrite(outPin, HIGH);
  }
  else {
    Serial.println("Uncovered");
    covered = LOW;
    digitalWrite(outPin, LOW);
  }
  delay(250);
}