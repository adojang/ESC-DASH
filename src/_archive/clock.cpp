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

#define NAME "clock"
#define MACAD 0xA2 // Refer to Table in Conventions


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

   if (d == "stop"){
    WebSerial.println("stop");
  ledcWrite(0, 0);
  ledcWrite(1, 0);
  }

  if (d == "trim1"){
    WebSerial.println("Trim 1");
    ledcWrite(0, 200);
  ledcWrite(1, 0);
  delay(5000);

  ledcWrite(0, 0);
  ledcWrite(1, 0);
  delay(1000);
  }

    if (d == "trim2"){
    WebSerial.println("Trim 2");
    ledcWrite(0, 0);
    ledcWrite(1, 200);
    delay(5000);

    ledcWrite(0, 0);
    ledcWrite(1, 0);
    delay(1000);
  }



}

/*************************WebSerial ****************************************/

VL53L0X sensor;

const int inPin = 15;
int servoPin = 13;

int sens1 = 0; 
int sens2 = 0; 
int sequence = 0;
unsigned long t = 0;
int servoPos = 0;

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x00}; // Address of Master Server DIRECTLY to CLOCK.
uint8_t setMACAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, MACAD};



/* ESP Async Timer */
AsyncTimer asynctimer(35);

/* WiFi Credentials */
const char* ssid = WIFI_SSID; // SSID
const char* password = WIFI_PASS; // Password

/* ESP-NOW Structures */
 dataPacket sData; // data to send
 dataPacket rData; // data to recieve

/* Setup */
AsyncWebServer server(80);
esp_now_peer_info_t peerInfo;

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

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&rData, incomingData, sizeof(rData));

  if(rData.origin == masterserver && rData.data == 66){
    ESP.restart();
  }

  if(rData.origin == masterserver && rData.data == 55){
    //Reset to original position
    reverseServo();

    //will not actually reverse unless clock is locked.
  }

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

void startWifi()
{
  /* Connect WiFi */
 WiFi.softAP(NAME, "pinecones", 0, 1, 4);
 WiFi.mode(WIFI_AP_STA);
  esp_wifi_set_mac(WIFI_IF_STA, &setMACAddress[0]);

  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);
  unsigned long wifitimeout = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    if ((millis() - wifitimeout) > 10000) ESP.restart();
  }
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("WiFi Failed!\n");
    return;
  }else
  {Serial.println("\n\nWIFI CONNECTED!");}

  if (!MDNS.begin(NAME))
  {
    Serial.println("Error setting up MDNS responder!");
    while (1)
    {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
 
  /* Elegant OTA */
  AsyncElegantOTA.begin(&server, "admin", "admin1234");

  /* WEB SERIAL REQUIRED TO FUNCTION */
  WebSerial.begin(&server);
  WebSerial.msgCallback(recvMsg);
  

  server.begin();
  MDNS.addService("http", "tcp", 80);
  WebSerial.println("mDNS responder started");
  WebSerial.println("WebSerial Service started");
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


}


const int PWM_pin1 = 4; // PWM Pin
const int PWM_pin2 = 5; // PWM Pin



void setup() {
  Serial.begin(115200);

  pinMode(PWM_pin1, OUTPUT);
  pinMode(PWM_pin2, OUTPUT);
  ledcAttachPin(PWM_pin1, 0);
  ledcAttachPin(PWM_pin2, 1);
  ledcSetup(0, 1000, 8);
  ledcSetup(1, 1000, 8);
  ledcWrite(0, 0);
  ledcWrite(1, 0);

  startWifi();
  startespnow();

  Wire.begin();
  sensor.init();
  sensor.setTimeout(500);
  sensor.startContinuous();

  pinMode(inPin, INPUT_PULLDOWN);
  pinMode(18,OUTPUT);
  digitalWrite(18,HIGH);

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


  delay(250); // changed from original 500;





  asynctimer.handle();
}