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
int resetPin = 13;
int countergreat = 0;
uint16_t sensorData = 0;

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x00}; // Address of Master Server
uint8_t setMACAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, MACAD};

/* ESP Async Timer */
AsyncTimer asynctimer(35);

void setup() {
  Serial.begin(115200);

  Wire.begin();
  sensor.init();
  sensor.setTimeout(500); // https://forum.pololu.com/t/vlx53l0x-timeout-issues/18553/15
  sensor.startContinuous();
  
  pinMode(outPin, OUTPUT);
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin,HIGH);

  
  }

unsigned long ttime = millis();
unsigned long hreset = millis();

void sensorReset(){
  //End Everything.
  sensor.stopContinuous();
  Wire.endTransmission();


    digitalWrite(resetPin, LOW);
    delay(1000);
    digitalWrite(resetPin, HIGH);
    delay(1000);
    //Init Sensor Again
    if (!sensor.init()) {
    Serial.println("Failed To Detect Sensor.. Restarting!!");
    ESP.restart();
  }

    sensor.setTimeout(500); // https://forum.pololu.com/t/vlx53l0x-timeout-issues/18553/15
    sensor.startContinuous();
}

void getSensorData(){
  static uint8_t counter = 0;

  sensorData = sensor.readRangeContinuousMillimeters();
  Serial.println(sensorData);

  if (sensorData < 100) {
    Serial.println("Covered");
    digitalWrite(outPin, HIGH);
    counter++;
  }
  else {
    //The Actual Value, see if its that massive error one.
    Serial.println(sensor.readRangeContinuousMillimeters());
    Serial.println("Uncovered");
      if(sensorData > 9000){
        counter++;
      }
    digitalWrite(outPin, LOW);
  }

  if(counter >= 20){
    Serial.println("RESET! Either Constant LOW or Constant HIGH Error for Sensor 1 Averted.");
    sensorReset();
    counter = 0;
  }


}

void loop()
{
//Option 1:

   if (millis() - ttime > 500){
    ttime = millis();
    getSensorData(); // Has Builtin Error detection.
}


  //Option 2:

  //Hardware Reset the sensor every 1 min. If this runs, I'll probably just leave it.
  // if(millis() - hreset > 60000){
  //   //Hardware Reset the Sensor
  //     hreset = millis();
  //     sensorReset();
  // }


 

}