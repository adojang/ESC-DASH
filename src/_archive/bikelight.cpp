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


#define NAME "bikelight"
#define MACAD 0xA1 // Refer to Table Below
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




// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x03}; // Address of Room Master
uint8_t setMACAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, MACAD};


/* ESP Async Timer */
AsyncTimer asynctimer;

/* WiFi Credentials */
const char* ssid = WIFI_SSID; // SSID
const char* password = WIFI_PASS; // Password

/* ESP-NOW Structures */






 dataPacket sData; // data to send
 dataPacket rData; // data to recieve

/* Setup */
AsyncWebServer server(80);
esp_now_peer_info_t peerInfo;
String success;

// bool opendoor1 = false;
// bool opendoor2 = false;
// bool opendoor3 = false;
// bool opendoor4 = false;



void triggerDoor(int pin, int timeout){
  digitalWrite(pin, LOW);
  Serial.println("Door Opened");
  asynctimer.setTimeout([pin]() {
      digitalWrite(pin, HIGH);
      Serial.println("Door Closed");
    }, 5000);
  
  
  // asynctimer.setInterval([]() {esp_now_send(broadcastAddress, (uint8_t *) &sData, sizeof(sData));},  5000);

}



/* ESP-NOW Callback Functions*/

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&rData, incomingData, sizeof(rData));

  Serial.println("Data Recieved...");
  

  // Add your code here to do something with the data recieved

}



 

void startwifi(){

  // Set device as a Wi-Fi Station
  WiFi.softAP(NAME, "pinecones", 0, 1, 4);
  WiFi.mode(WIFI_AP_STA);
  esp_wifi_set_mac(WIFI_IF_STA, &setMACAddress[0]);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.printf("WiFi Failed!\n");
      ESP.restart();
      return;
  }
  else{
    Serial.println("WIFI CONNECTED!");
  }

   /* MDNS */
  if (!MDNS.begin(NAME)) {
        Serial.println("Error setting up MDNS responder!");
        while(1) {
            delay(1000);
        }
    }
  Serial.println("mDNS responder started");
  Serial.printf("*** PROGRAM START ***\n\n");
  
  AsyncElegantOTA.begin(&server, "admin", "admin1234");

  server.begin();
  MDNS.addService("http", "tcp", 80);

}

void startespnow(){
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  //Register Callback Functions
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  else{Serial.print("Peer Added");}

}

int normalDelay = 650;
int longDelay = normalDelay * 2;
void shortlight(){

  ledcWrite(0, 4000);
  digitalWrite(2,HIGH);
  delay(normalDelay);
  ledcWrite(0, 0);
  digitalWrite(2,LOW);
  delay(normalDelay);
}
void longlight(){
  ledcWrite(0, 4000);
  digitalWrite(2,HIGH);
  delay(longDelay);
  digitalWrite(2,LOW);
  ledcWrite(0, 0);
  delay(normalDelay);

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


 void sendData(){

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sData, sizeof(sData));
  Serial.println("Data Try to Send");
  if (result == ESP_OK) { Serial.println("Sent Data success");}
  else {Serial.println("Error sending the data");}

 }


void setup() {
  Serial.begin(115200);
  startwifi();
  startespnow();


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

        asynctimer.setInterval([]() {
      sendData();
    }, 250);
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
  }

  if(currentState==HIGH){
    digitalWrite(INTERNAL_LED, HIGH);
  } else {
    digitalWrite(INTERNAL_LED, LOW);
  }




  asynctimer.handle();
}