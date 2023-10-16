/*
--------------------------------------------------------------------------
                          Tygervallei Escape Room Project
--------------------------------------------------------------------------                          
  Author: Adriaan van Wijk
  Date: 16 October 2023

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

#define NAME "thumbreader"
#define setMACAddress m_train_thumb

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

unsigned long ttime = millis();
// Thumb Variables
float OPENVALUE = 30;
float tuneconstant = 0.84; // Value to start with. Callibration should sort this out.

//0.75 only had 3 errors during a hole day, increasing it to 0.84
float samples1[50];
float averages[10] = {40,40,40,40,40,40,40,40,40,40};
int val = 0;
float sum;
int avg1=40;
int avg2=40;
int ScannedSuccessful=0;
unsigned long locktimer = millis();


float currentValue = 0.0;
float previousEMA = 0.0;
float prevousEMA2 = 0.0;
float smoothingFactor = 0.8;  // Adjust this value based on your application
//original value was 0.8

/* Functions */
void opensesame(int pin){
  digitalWrite(pin, LOW);
  Serial.println("Door Opened");
  WebSerial.println("Door Opened");
  asynctimer.setTimeout([pin]() {
      digitalWrite(pin, HIGH);
      Serial.println("Door Closed");
    }, 3000);
}




void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // WebSerial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");
  }

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) 
{
  memcpy(&rData, incomingData, sizeof(rData));

  if((rData.origin == masterserver) && (rData.sensor == masterserver) && (rData.data == 1)){
    Serial.println("Override Button Pressed");
    WebSerial.println("Override Button Pressed");
    opensesame(13); // Open the door
  }

    if((rData.origin == masterserver) && (rData.sensor == masterserver) && (rData.data == 66)){
    Serial.println("ESP32 RESTART and RECALLIBRATE");
    WebSerial.println("ESP32 RESTART and RECALLIBRATE");
    delay(500);
    ESP.restart();
  }

}

float emaFilter(float currentValue, float previousEMA, float smoothingFactor) {
  return (currentValue * smoothingFactor) + (previousEMA * (1 - smoothingFactor));
}

float absolute(float value){
  if (value < 0){
    return -value;
  } else
  return value;
}

void callibrateReader(){
for (int j=0; j<100; j++){
  val = hallRead();
  sum=sum + val;
  delay(10);
}
sum = sum/100;
OPENVALUE = emaFilter(sum, previousEMA, smoothingFactor);
WebSerial.printf("Thumb Reader Callibrated OpenValue is ");
WebSerial.println(OPENVALUE);
Serial.println("Baseline Value is: ");
Serial.println(OPENVALUE);

//Test if it triggers at all:
bool continuecalibration = true;
locktimer = millis();
while(continuecalibration){

  for (int j=0; j<20; j++){
    val = hallRead();
    sum=sum + val;
  }
  sum = sum/20;
  float filteredValue = emaFilter(sum, previousEMA, smoothingFactor);
  previousEMA = filteredValue;

  if (absolute(OPENVALUE - filteredValue) > (tuneconstant*OPENVALUE) && ((millis() - locktimer) > 100)){
    Serial.printf("Threshhold Adjusted: %f\n", OPENVALUE*tuneconstant);
    WebSerial.printf("Threshhold Adjusted: %f\n", OPENVALUE*tuneconstant);
    locktimer = millis();
    tuneconstant = tuneconstant + 0.15*tuneconstant; // increase by 10% each time.
  }

  if (millis() - locktimer > 10000){
    continuecalibration = false; // We have reached equilibrum, no triggering for 5 seconds
    Serial.printf("\n");
    tuneconstant = tuneconstant*1.25; // SLIGHT increase from baseline. Increased here based on feedback from Wendy from 1.2 to 1.25
    Serial.printf("Final Threshhold: %f\n", OPENVALUE*tuneconstant);
    WebSerial.printf("Final Threshhold: %f\n", OPENVALUE*tuneconstant);
  }


}

  
Serial.println("Sucessfully Callibrated");
WebSerial.println("Sucessfully Callibrated");

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
  sData.origin = train_thumb;
  sData.sensor = status_alive;
  esp_err_t result = esp_now_send(m_masterserver, (uint8_t *) &sData, sizeof(sData));
}


void setup() {
  Serial.begin(115200);

  Core.startup(setMACAddress, NAME, server);
  startespnow();
  registermac(m_masterserver);

  sData.origin = train_thumb;
  sData.sensor = train_thumb;

  pinMode(2, OUTPUT);
  pinMode(13,OUTPUT); // Relay Trigger
  pinMode(18,OUTPUT);
  digitalWrite(18,LOW);
  pinMode(5,OUTPUT);
  digitalWrite(5,HIGH);
  digitalWrite(13,HIGH);
  delay(2000);
  Serial.println("Callibrate Reader");
  WebSerial.println("Callibrate Reader");
  callibrateReader();
  ttime = millis();
  locktimer = millis();


  asynctimer.setInterval([]() {statusUpdate();},  1000);

}



void loop() {

//Take 25 Readings and Average Them.
for (int j=0; j<20; j++){
  val = hallRead();
  sum=sum + val;
}
sum = sum/20;
float filteredValue = emaFilter(sum, previousEMA, smoothingFactor);
previousEMA = filteredValue;
  
if (millis() - ttime > 2000){
  ttime = millis();
  // WebSerial.printf("Absolute Read Value: %f\n", (absolute(filteredValue)));
  // WebSerial.printf("Threshold Value: %f\n", OPENVALUE*tuneconstant);
  WebSerial.printf("Read Value > ThreshHold [%f > %f]  \n",absolute(OPENVALUE - filteredValue), tuneconstant*OPENVALUE);

  Serial.println(absolute(OPENVALUE - filteredValue));
}

//Open value is my resting, non-finger state. filtered value is my finger being there.

if (absolute(OPENVALUE - filteredValue) > tuneconstant*OPENVALUE && ((millis() - locktimer) > 2000)){
  WebSerial.printf("OpenValue: %f\n", OPENVALUE);
  WebSerial.printf("filteredValue: %f\n", filteredValue);
  WebSerial.printf("Threshhold: %f\n", OPENVALUE*tuneconstant);
  WebSerial.printf("Difference: %f\n", absolute(OPENVALUE - filteredValue));
  locktimer = millis();;
  WebSerial.println("OpenSesame");
  opensesame(13); // 5s timeout til resets.
}


  asynctimer.handle();
}