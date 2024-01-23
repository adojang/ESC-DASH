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
#include <esp_task_wdt.h> // watchdog for doorlock mag recovery.
#include <EEPROM.h>
#define NAME "clock"
#define setMACAddress m_clock

//Watchdog Woof Woof
#define WDT_TIMEOUT 15 // 5 seconds

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

// AsyncTimer asynctimer;
AsyncWebServer server(80);
ESPDash dashboard(&server,false);
esp_now_peer_info_t peerInfo;
EscCore Core;

dataPacket sData; // data to send
dataPacket rData; // data to recieve

/* Configuration and Setup */

const int inPin = 15;
const int servoPin = 13;

bool oneShot = false;
bool readOneShot = true;
bool RecvShot = false;
bool reverse = false;
bool forward = false;
int sens1 = 0; 
int sens2 = 0; 
int sequence = 0;
unsigned long t = 0;
int servoPos = 0;
uint16_t sensorData = 0;
const int resetPin = 13;
const int PWM_pin1 = 4; // PWM Pin
const int PWM_pin2 = 5; // PWM Pin
unsigned long stoptimer = millis();
const int lightpin = 16;

/* Functions */


void activateServo(){

  digitalWrite(lightpin,HIGH);
  // 7.5mm per second speed
  // 6 seconds to go 45mm.
  ledcWrite(0, 200);
  ledcWrite(1, 0);
  delay(11300);
  esp_task_wdt_reset();

  ledcWrite(0, 0);
  ledcWrite(1, 0);
  delay(1000);
}

void reverseServo(){
  digitalWrite(lightpin,LOW);
  Serial.println("reverseservo");
  ledcWrite(0, 0);
  ledcWrite(1, 200);
  delay(7700);
  esp_task_wdt_reset();

  ledcWrite(0, 0);
  ledcWrite(1, 0);
  delay(1000);
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // WebSerial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");
  }

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&rData, incomingData, sizeof(rData));



    if(rData.origin == masterserver && rData.data == 55){
      reverse = true;

    //will not actually reverse unless clock is
  }

     if(rData.origin == masterserver && rData.data == 42){
      forward = true;

  }


  if(rData.origin == masterserver && rData.data == 10){
  ledcWrite(0, 200);
  ledcWrite(1, 0);
  delay(1000);

  ledcWrite(0, 0);
  ledcWrite(1, 0);
  delay(1000);
  }

  if(rData.origin == masterserver && rData.data ==20){
    ledcWrite(0, 0);
    ledcWrite(1, 200);
    delay(1000);

    ledcWrite(0, 0);
    ledcWrite(1, 0);
    delay(1000);
  }

if(rData.origin == masterserver && rData.sensor == attic_clock){
  //Max / Min +- 60

  // int speeddata = rData.data*3.335;
int speeddata = rData.data*4.25;
  Serial.printf("Seed Data %d\n", speeddata);

  if (rData.data > 0){
    ledcWrite(0, speeddata);
    ledcWrite(1, 0);
    Serial.println("Going A");
    stoptimer = millis();
  }

  if(rData.data < 0){
    ledcWrite(0, 0);
    ledcWrite(1, -speeddata);
    Serial.println("Going B");
    stoptimer = millis();
}
}

  // }


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
  sData.origin = attic_clock;
  sData.sensor = status_alive;
  esp_err_t result = esp_now_send(m_masterserver, (uint8_t *) &sData, sizeof(sData));
  Serial.println(result == ESP_OK ? "Status Update Sent" : "Status Update Failed");
  esp_task_wdt_reset();
}


void setup() {
  Serial.begin(115200);
  EEPROM.begin(24);

  bool temp = EEPROM.read(0);
  Serial.println(temp);
  oneShot = temp;
  Serial.println(oneShot);
  Serial.println("Configuring WDT...");
  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch

  //This needs to start first to prevent motor drift
  pinMode(PWM_pin1, OUTPUT);
  pinMode(PWM_pin2, OUTPUT);
  ledcAttachPin(PWM_pin1, 0);
  ledcAttachPin(PWM_pin2, 1);
  ledcSetup(0, 1000, 8);
  ledcSetup(1, 1000, 8);
  ledcWrite(0, 0);
  ledcWrite(1, 0);

//LED Light Power Point source
pinMode(25,OUTPUT);
digitalWrite(25,HIGH);

//Triggering PIN, 0 = active, 1 = off
  pinMode(lightpin, OUTPUT);
  digitalWrite(lightpin,LOW);


  pinMode(23,OUTPUT);
  digitalWrite(23,LOW);

    pinMode(15, INPUT); //sensor 1
    pinMode(23, INPUT); // sensor1

  pinMode(resetPin, OUTPUT);
  pinMode(resetPin, HIGH);

  Core.startup(setMACAddress, NAME, server);
  startespnow();
  registermac(m_masterserver);
  registermac(m_atticmaster);

  pinMode(2,OUTPUT);
  digitalWrite(2,HIGH); // PRove that Wifi is Connected Rightly.
  sData.origin = attic_clock;
  sData.sensor = attic_clock;


  pinMode(inPin, INPUT_PULLDOWN);
  pinMode(18,OUTPUT);
  digitalWrite(18,HIGH);

//Set Timeout Occured to 0;
    sData.origin = attic_clock;
    sData.sensor = attic_clock;
    sData.data = 90;
    esp_err_t result = esp_now_send(m_masterserver, (uint8_t *) &sData, sizeof(sData));
    

  // asynctimer.setInterval([]() {statusUpdate();},  1000);

}

void getReadings() {
  static uint8_t counter = 0;

  sens1 = digitalRead(15);
  sens2 = digitalRead(23);



  // Serial.printf("sensor 1: %d\n", sens1);
  // WebSerial.printf("sensor 1: %d\n", sens1);

  // Serial.printf("sensor 2: %d\n", sens2);
  // WebSerial.printf("sensor 2: %d\n", sens2);
  
  // Serial.println(sensor.readRangeContinuousMillimeters());
}

void printSequence() {
  Serial.printf("Sequence: %d\n", sequence);
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println(info.wifi_sta_disconnected.reason);
  Serial.println("Trying to Reconnect");
}

unsigned long ttime = millis();
unsigned long gtime = millis();
  //Sense1 is where the hole will be.
  //Sense2 is where the hole is covered.

// int counter = 0;
// int clocktimer = millis();

void triggersystem(){
      oneShot = true;
      EEPROM.write(0, true);
      EEPROM.commit();
      Serial.println(EEPROM.read(0));
      sData.origin = attic_clock;
      sData.sensor = attic_clock;
      if (oneShot == true) sData.data = 1;
      if (oneShot == false) sData.data = 0;
      esp_now_send(m_atticmaster, (uint8_t *) &sData, sizeof(sData));
      WebSerial.println("Send Oneshot. Clock now thinks it is in UP position.");
      Serial.println(oneShot);
      activateServo();
}

/// @brief This keeps track of the transients. It increases by 1 in 250ms intervals when true, and decays by 1 in 1000ms intervals when false.
int clockcount = 0;

void loop() {

// if (millis() - clocktimer > 30000){
//   clocktimer = millis();
//   counter += 1;
//   WebSerial.printf("Count of Successfull UP/DOWN sequences: %d\n", counter);
//   //trigger clock up and down... Will not write to EEPROM.
//   activateServo();
//   delay(10000);
//   reverseServo();
// }

  if(millis()-gtime > 1000){
    gtime = millis();
    statusUpdate();
    Serial.println("Post Status Update");
    clockcount--;
    if (clockcount < 0) clockcount = 0;
    WebSerial.println(clockcount);
  }


  if((millis() - stoptimer > 250) && (sequence == 0)){
    ledcWrite(0, 0);
    ledcWrite(1, 0);
    stoptimer = millis();
}


  //ReadSensor and Handle Timeouts and check the Sequence and Trigger the Motor.
  //increased from 500
  if (millis() - ttime > 500){
    ttime = millis();
  
  getReadings();

    if ((sens1==HIGH) && (sens2==LOW)) { // correct holes covered, trigger
      clockcount++;
      WebSerial.println("Count In:");
      WebSerial.println(clockcount);
    }

  }

if (clockcount >= 3 && (oneShot == false)){
  WebSerial.println("TRIGGER!");
  clockcount = 0;
  triggersystem();
}




  //Motor Reverse Code that is called through the masterserver.
  if(reverse == true){
    WebSerial.println("REVERSE = TRUE so Write to EEPROM");
    EEPROM.write(0, false);
    EEPROM.commit();
    
    oneShot = false;
    reverse = false;
    // sData.origin = attic_clock;
    // sData.sensor = attic_clock;
    // if (oneShot == true) sData.data = 1;
    // if (oneShot == false) sData.data = 0;
    // esp_now_send(m_atticmaster, (uint8_t *) &sData, sizeof(sData));
    // Serial.println("Send Oneshot");
    // Serial.println(oneShot);
    //Reset to original position
    reverseServo();

    //Need to update master
  
  }


    if(forward == true){
    WebSerial.println("FORWARD = TRUE so Write to EEPROM");
    EEPROM.write(0, true);
    EEPROM.commit();
    oneShot = true;
    forward = false;
 
    activateServo();

    //Need to update master
  
  }

//Execute one on Boot after a msg has been recieved.
  // if (readOneShot && (millis() - gtime > 4000)){
  //   Serial.println("Read OneShot");
  //   Serial.println(RecvShot);
  //   oneShot = RecvShot;
  //   readOneShot = false;
  // }

  //Wifi Reconnecting Code
  if (WiFi.isConnected() == false){
  Serial.println("Process Wifi Start");
  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  
  Serial.println("Start Reconnect Process");
  WiFi.softAPdisconnect();
  WiFi.disconnect();

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  delay(500);
  while(!WiFi.isConnected()){
    Serial.println("Waiting...");
    delay(200);
  }
}


  // asynctimer.handle();
}