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
#include <Wire.h>
#include <VL53L0X.h>

#define NAME "clock"
#define setMACAddress m_clock

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
VL53L0X sensor;

const int inPin = 15;
int servoPin = 13;

int sens1 = 0; 
int sens2 = 0; 
int sequence = 0;
unsigned long t = 0;
int servoPos = 0;

const int PWM_pin1 = 4; // PWM Pin
const int PWM_pin2 = 5; // PWM Pin


/* Functions */
void activateServo(){
  // 7.5mm per second speed
  // 6 seconds to go 45mm.
  ledcWrite(0, 200);
  ledcWrite(1, 0);
  delay(12000);

  ledcWrite(0, 0);
  ledcWrite(1, 0);
  delay(1000);
}

void reverseServo(){
  ledcWrite(0, 0);
  ledcWrite(1, 200);
  delay(12000);

  ledcWrite(0, 0);
  ledcWrite(1, 0);
  delay(1000);
}



void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {WebSerial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&rData, incomingData, sizeof(rData));


  if(rData.origin == masterserver && rData.data == 10){
  ledcWrite(0, 200);
  ledcWrite(1, 0);
  delay(2000);

  ledcWrite(0, 0);
  ledcWrite(1, 0);
  delay(1000);
  }

  if(rData.origin == masterserver && rData.data ==20){
        WebSerial.println("Trim 2");
    ledcWrite(0, 0);
    ledcWrite(1, 200);
    delay(2000);

    ledcWrite(0, 0);
    ledcWrite(1, 0);
    delay(1000);
  }
  // if (rData.origin == masterserver && rData.sensor == masterserver){
  //   if(rData.data == 1) {activateServo();} // Manually Trigger Clock Servo

  //   if(rData.data == 2) {reverseServo();} // Reverse Servo to Start
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
}


void setup() {
  Serial.begin(115200);

  //This needs to start first to prevent motor drift
  pinMode(PWM_pin1, OUTPUT);
  pinMode(PWM_pin2, OUTPUT);
  ledcAttachPin(PWM_pin1, 0);
  ledcAttachPin(PWM_pin2, 1);
  ledcSetup(0, 1000, 8);
  ledcSetup(1, 1000, 8);
  ledcWrite(0, 0);
  ledcWrite(1, 0);


  Core.startup(setMACAddress, NAME, server);
  startespnow();
  registermac(m_masterserver);
  registermac(m_atticmaster);

  sData.origin = attic_clock;
  sData.sensor = attic_clock;

  Wire.begin();
  sensor.init();
  sensor.setTimeout(500);
  sensor.startContinuous();

  pinMode(inPin, INPUT_PULLDOWN);
  pinMode(18,OUTPUT);
  digitalWrite(18,HIGH);

  asynctimer.setInterval([]() {statusUpdate();},  1000);

}


void getReadings() {
  
  sens2 = digitalRead(inPin);

  if (sensor.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

  if (sensor.readRangeContinuousMillimeters()<100) {
    sens1 = HIGH;
  }
  else {
    sens1 = LOW;
  }

  Serial.print("sensor 1: ");
  Serial.print(sens1);
  Serial.print(", sensor 2: ");
  Serial.println(sens2);
  Serial.println(sensor.readRangeContinuousMillimeters());
}

void printSequence() {
  Serial.print("Sequence: ");
  Serial.println(sequence);
}

void loop() {
  
  getReadings();
  // printSequence();

  //Sense1 is where the hole will be.
  //Sense2 is where the hole is covered.

  if ((sequence==0) && (sens1==HIGH) and (sens2==HIGH)){ // both are initially open.
    sequence=1;
  }
  
  if ((sequence==1) and (sens1==LOW) and (sens2==HIGH)) { // correct holes covered, trigger
    sequence=0;
    activateServo();
  } 
  else if ((sequence==1) and (sens1==LOW) and (sens2==LOW)) { // both are open
    sequence=0;
  }
  printSequence();

// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING 
  delay(250); // changed from original 500;
  //This delay function must be removed for accurate high speed sampling...

  //WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING 


  asynctimer.handle();
}