/*
--------------------------------------------------------------------------
                          Tygervallei Escape Room Project
--------------------------------------------------------------------------                          
  Author: Adriaan van Wijk
  Date: 24 January 2024

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


#define NAME "bike"
#define setMACAddress m_attic_bike

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
#define INTERNAL_LED 2
#define SWITCH_PIN 15 
#define ARRAY_LENGTH 3

int previousState;
int currentState;
bool measureTime;
int val = 0;
float dt = 0;
float t = 0;
float freq = 0;
unsigned long switchTime = 0;
unsigned long decayTime1 = 0;
unsigned long decayTime2 = 0;
unsigned long transmitTime = 0;

int i = 0;
float dtMeasurements[ARRAY_LENGTH] = {10000, 10000, 10000};
int measurementIndex = 0;
float dtAvg = 0;
float freqAvg = 0;
float freqNorm = 0;
float output = 0;

/* Functions */

float calculateAverage() {
    float sum = 0;
    int count = 0;
    for (int i = 0; i < ARRAY_LENGTH; i++) {
      if (dtMeasurements[i]>100) {
          sum += dtMeasurements[i];
          count++;
      }
    }
    return sum / count;
}

// Function to take a measurement and update the array
void updateMeasurements(float measurement) {
    // Add the new measurement to the array
    dtMeasurements[measurementIndex] = measurement;

    // Update the index of the next measurement, wrapping around if necessary
    measurementIndex = (measurementIndex + 1) % ARRAY_LENGTH;
}

void printArray(float* array, int length) {
    for (int i = 0; i < length; i++) {
        printf("%.2f ", array[i]);
    }
    printf("\n");
}

float normalise(float output) {
    if (output>2) {
        output=2;
    }
    output = output/1.2;
    return output;
}


 void sendData(){
  sData.origin = attic_bike;
  sData.sensor = attic_bike;
  esp_err_t result = esp_now_send(m_atticmaster, (uint8_t *) &sData, sizeof(sData));
  // Serial.println(sData.data);
 }


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {

  // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&rData, incomingData, sizeof(rData));

  Serial.println("Data Recieved...");
  

  // Add your code here to do something with the data recieved

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
  sData.origin = attic_bike;
  sData.sensor = status_alive;
  esp_err_t result = esp_now_send(m_masterserver, (uint8_t *) &sData, sizeof(sData));
}


void setup() {
  Serial.begin(115200);



  Core.startup(setMACAddress, NAME, server);
  startespnow();
  registermac(m_masterserver);
  registermac(m_atticmaster);

  sData.origin = attic_clock;
  sData.sensor = attic_clock;

  pinMode(INTERNAL_LED, OUTPUT);
  pinMode(SWITCH_PIN, INPUT_PULLDOWN);


  currentState = digitalRead(SWITCH_PIN);
  previousState = currentState;
  t = millis();
  measureTime = false;
  transmitTime = millis();

  sData.origin = attic_bike;
  sData.sensor = attic_bike;
  sData.data = 0;

  asynctimer.setInterval([]() {sendData();}, 150);

  asynctimer.setInterval([]() {statusUpdate();},  1000);

}


unsigned long ttime = millis();
void loop() {

  previousState = currentState;
  currentState = digitalRead(SWITCH_PIN);
  

  if((currentState==HIGH) && (previousState==LOW)) {
    switchTime = millis();
    measureTime = true;
  }

  if((currentState==HIGH) && ((millis()-switchTime)>5) && (measureTime==true)) {
    dt = millis() - t;
    t = millis();
    decayTime1 = millis();
    decayTime2 = millis();
    // freq = 1/((float)dt/1000);

    updateMeasurements(dt);
    dtAvg = calculateAverage();
    freqAvg = 1000/dtAvg;
    freqNorm = normalise(freqAvg);

    measureTime=false;
    i++;

  } 

  if ((millis()-decayTime1)>1500) {
    if ((millis()-decayTime2)>250){
        decayTime2 = millis();
        freqNorm=freqNorm*0.7;
        // Serial.println(freqNorm);
    }
  }

  if (millis()-transmitTime>250) {
      transmitTime=millis();
      output=freqNorm;
      // Serial.println(output);
      sData.data = int(output*100);
      

      if(sData.data == 166){
        //Edge case trying to detect anything that's outside of the range we want.
        sData.data = 0;
      }

      if (sData.data > 100) sData.data = 100; // Ensures our output scales from 0 to 100
      Serial.println(sData.data);

  }

  // if(currentState==HIGH){
  //   digitalWrite(INTERNAL_LED, HIGH);
  // } else {
  //   digitalWrite(INTERNAL_LED, LOW);
  // }

  
  asynctimer.handle();
}