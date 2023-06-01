/*
  --------------------------------------------------------------------------
  Escape Room Template
  Adriaan van Wijk
  22 May 2023

  Give a short explaination of what this code does and for what puzzle it is.

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

#define NAME "bikelight"
#define MACAD 0x02 // Refer to Table Below
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


/* Data Naming Convention for Mac Addresses
*  0x00 - masterserver
 * 0x01 - humanchain
 * 0x02 - bikelight
 * 0x03 - clockmotor
 * 0x04 - beetle
 * 0x05 - chalicedoor
 * 0x06 - ringreader
 * 0x07 - tangrumtomb
 * 0x08 - thumbreaderdoor
*/

/* Kernal*/
#include <Arduino.h>
#include <config.h>

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

/* SET MAC ADDRESS */
uint8_t broadcastAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x00}; // Address of Master Server
uint8_t setMACAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, MACAD};

/* ESP Async Timer */
AsyncTimer asynctimer;

/* WiFi Credentials */
const char* ssid = WIFI_SSID; // SSID
const char* password = WIFI_PASS; // Password

/* ESP-NOW Structures */
typedef struct dataPacket {
int trigger = 0;
} dataPacket;

dataPacket sData; // data to send
dataPacket rData; // data to recieve

/* Setup */
AsyncWebServer server(80);
esp_now_peer_info_t peerInfo;


// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");

}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&rData, incomingData, sizeof(rData));
  Serial.println("Override Data Recieved...");

  //Incoming Data is copied to rData. Do something with it here or in the main loop.
  //Incoming Data Goes Here
}

void startespnow(){
    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
      return;
    }

    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(OnDataSent);
    
    // Register peer
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    
    // Add peer        
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
      Serial.println("Failed to add peer");
      return;
    }
    // Register for a callback function that will be called when data is received
    esp_now_register_recv_cb(OnDataRecv);
}

void startup(){
  /* Connect WiFi */
  WiFi.mode(WIFI_STA);
  esp_wifi_set_mac(WIFI_IF_STA, &setMACAddress[0]);
  
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
  // }
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.printf("WiFi Failed!\n");
      return;
  }

  /* MDNS */
  if (!MDNS.begin(NAME)) {
        Serial.println("Error setting up MDNS responder!");
        while(1) {
            delay(1000);
        }
    }
  Serial.println("mDNS responder started");
  
  AsyncElegantOTA.begin(&server, "admin", "admin1234");

  server.begin();
  MDNS.addService("http", "tcp", 80);

}


// Function to calculate the average of an array excluding the outlier value
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

// Normalises output to (0,1), assuming the highest possible cadence is 2Hz.
float normalise(float output) {
    if (output>2) {
        output=2;
    }
    output = output/2;
    return output;
}


 


void setup() {
    Serial.begin(115200);
    startup(); // Startup for Wifi, mDNS, and OTA
    startespnow(); // Startup for ESP-NOW

    //Begin Sending Data to Remote ESP's every 250ms
    // asynctimer.setInterval([]() { esp_now_send(broadcastAddress, (uint8_t *) &sData, sizeof(sData));},  250);

    pinMode(INTERNAL_LED, OUTPUT);
    pinMode(SWITCH_PIN, INPUT_PULLDOWN);


    currentState = digitalRead(SWITCH_PIN);
    previousState = currentState;
    t = millis();
    measureTime = false;
    transmitTime = millis();
  }


void loop() {
  //Insert Code Here
  //You need to edit OnDataRecv to handle incoming overrides.
  // Serial.println("Looping");

  previousState = currentState;
  currentState = digitalRead(SWITCH_PIN);
  
  // val = hallRead();
  // // Serial.println(val);

  // if (val<0){
  //   currentState = HIGH;
  // } else {
  //   currentState = LOW;
  // }


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

    // Serial.println(i);
    // Serial.println(dt);
    // Serial.println(freq);
    // printArray(dtMeasurements, ARRAY_LENGTH);
    // Serial.println(dtAvg);
    // Serial.println(freqAvg);
    // Serial.println(freqNorm);
  } 

  if ((millis()-decayTime1)>2000) {
    if ((millis()-decayTime2)>250){
        decayTime2 = millis();
        freqNorm=freqNorm*0.9;
        Serial.println(freqNorm);
    }
  }

  if (millis()-transmitTime>250) {
      transmitTime=millis();
      output=freqNorm;
      Serial.println(output);
  }

  if(currentState==HIGH){
    digitalWrite(INTERNAL_LED, HIGH);
  } else {
    digitalWrite(INTERNAL_LED, LOW);
  }

  asynctimer.handle();
}



