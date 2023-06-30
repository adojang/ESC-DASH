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

#define NAME "atticmaster"
#define MACAD 0x03 // Refer to Table in Conventions


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


// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x00}; // Address of Master Server
uint8_t m_RFID1[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA4}; // Address of RFID1 Server
uint8_t m_RFID2[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA5}; // Address of RFID2 Server
uint8_t m_RFID3[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA6}; // Address of RFID3 Server
uint8_t m_RFID4[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA7}; // Address of RFID4 Server
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
String success;
int emergencyFlag = 0;
int emergencyTrigger = 0;
int timeout2 = 0;
const unsigned long emergencyButtonTimeout = 5000;

bool RFID1_status = false;
bool RFID2_status = false;
bool RFID3_status = false;
bool RFID4_status = false;

bool RFID1_armed = false;


bool DOORTOUCH = false;
bool doorlocked = false;

int readingcounter = 0;

float currentValue = 0.0;
float previousEMA = 0.0;
float prevousEMA2 = 0.0;

float smoothingFactor = 0.8;  // Adjust this value based on your application //0.8 600-3400 floating, 4000 touching

float emaFilter(float currentValue, float previousEMA, float smoothingFactor) {
  return (currentValue * smoothingFactor) + (previousEMA * (1 - smoothingFactor));
}

void getTouch(){

  int reading = analogRead(34);
  float filteredValue = emaFilter(reading, previousEMA, smoothingFactor);
  previousEMA = filteredValue;
  sData.data = filteredValue;
  sData.origin = attic_humanchain;
  sData.sensor = attic_humanchain;


  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sData, sizeof(sData));

  if(filteredValue > 3800){
    Serial.printf("ABOVE THRESHOLD\n\n\n\n");
    readingcounter +=80; // trigger in 2 ticks.
  }

  if (readingcounter >= 100) {
    readingcounter = 0;
    Serial.printf("TRIGGERD THRESHOLD\n");
    Serial.println("Human Chain Touch Detected!");
    WebSerial.printf("Touch Detected!\n\n");
    DOORTOUCH = true;
    sData.origin = atticmaster;
    sData.sensor = attic_humanchain;
    sData.data = 1;
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sData, sizeof(sData));
  }
  
  readingcounter -= 5;
  if (readingcounter < 0) readingcounter = 0;
}


void triggerDoor(int pin, int timeout){
  digitalWrite(pin, LOW);
  Serial.println("Door Opened");
  // delay(15);
  asynctimer.setTimeout([pin]() {
      digitalWrite(pin, HIGH);
      Serial.println("Door Closed");
    }, 5000);
}

void IRAM_ATTR emergency(){
  emergencyFlag = 1;
}

/* ESP-NOW Callback Functions*/

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&rData, incomingData, sizeof(rData));

  if(rData.origin == masterserver && rData.sensor == masterserver){
    if (rData.data == 88) doorlocked = true; // locked
    if(rData.data == 99) doorlocked = false; // unlocked
  }

    //RESET THE WHOLE ROOM.
  if ((rData.origin == masterserver) && (rData.data == 66)){
    //Restart all other RFID sensors?
    sData.origin = atticmaster;
    sData.sensor = atticmaster;
    sData.data = 66;
    WebSerial.println("Order 66 Restarting.");
    delay(250);
    esp_err_t result = esp_now_send(m_RFID4, (uint8_t *) &sData, sizeof(sData));
    delay(250);
    ESP.restart();
  }

  
  // if(rData.origin == attic_morse && rData.sensor == attic_morse && rData.data == 1){
  //   RFID2_status = false;
  //   RFID3_status = false;
  //   RFID4_status = false;
    
  //   //Restart the Sequence.

  // }


  // Forward Data from Sensors to Master Server
  if (rData.origin != masterserver){
        esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &rData, sizeof(rData));
  }

  /*    STATUS UPDATES    */

    if(rData.origin == attic_RFID1 && rData.sensor == attic_RFID1 && rData.data == 1){
    RFID1_status = true;
    //Send an Updated 'flag' to master
    rData.data = 100;
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &rData, sizeof(rData));
  }



  if(rData.origin == attic_RFID2 && rData.sensor == attic_RFID2 && rData.data == 2){
    RFID2_status = true;
    //Send an Updated 'flag' to master
    rData.data = 100;
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &rData, sizeof(rData));
  }


  if(rData.origin == attic_RFID3 && rData.sensor == attic_RFID3 && rData.data == 3){
    RFID3_status = true;
    //Send an Updated 'flag' to master
    rData.data = 100;
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &rData, sizeof(rData));
  }


    //RFID 4 is 5/6 because one of them is a bit wonky.
    if(rData.origin == attic_RFID4 && rData.sensor == attic_RFID4 && rData.data >= 6){
    RFID4_status = true;
    //Send an Updated 'flag' to master
    rData.data = 100;
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &rData, sizeof(rData));
  }


    if (rData.origin == masterserver && rData.sensor == attic_humanchain && rData.data){
      triggerDoor(5, 2000);
      triggerDoor(18, 2000);
      triggerDoor(19, 2000);
      triggerDoor(21, 2000);
    }

    if (rData.origin == masterserver && rData.sensor == attic_bike){
      if (rData.data == 1) ledcWrite(1, (4000));

      if(rData.data == 0) ledcWrite(1, (4000));
    }



     if(rData.origin == attic_bike && rData.sensor == attic_bike)
    {
    //Send Data to LED to light up appropriately
      //4000 / 100 = 40
      ledcWrite(1, ((40)*rData.data));
      // Serial.println(rData.data);

    }


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

/* RFID Peers */

    memcpy(peerInfo.peer_addr, m_RFID4, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
      WebSerial.println("Failed to add peer");
      return;
    }



}


void setup() {
  Serial.begin(115200);
  startWifi();
  startespnow();
  //Pin Setups

  pinMode(22, OUTPUT); // emergency Button GND
  digitalWrite(22, HIGH);
  pinMode(23, INPUT_PULLDOWN); // emergency Button
  pinMode(2, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(18, OUTPUT);
  pinMode(19, OUTPUT);
  pinMode(21, OUTPUT);
  digitalWrite(5, HIGH);
  delay(250);
  digitalWrite(18, HIGH);
  delay(250);
  digitalWrite(19, HIGH);
  delay(250);
  digitalWrite(21, HIGH);

 attachInterrupt(17, emergency, HIGH);

  // BIKE LIGHT
  ledcSetup(1, 100, 12); // Bike LED
  ledcAttachPin(26, 1);
  ledcWrite(1, 0);

  analogReadResolution(12);

  //GND Pin for PWM Controllers
  pinMode(33, OUTPUT);
  digitalWrite(33, LOW);
  pinMode(25, OUTPUT);
  digitalWrite(25, LOW);

  //HumanTouch Pins
  pinMode(15, INPUT_PULLDOWN); // Goes to ESP32 atticmaster2 GPIO output.
  // pinMode(15, OUTPUT);
  // digitalWrite(15,HIGH);

emergencyTrigger = millis();
timeout2 = millis();
}


void loop() {
  
  //HumanChain Logic
if ((DOORTOUCH == true) && (RFID2_status == true) && (RFID3_status == true) && (RFID4_status == true) && (doorlocked == false)){
  //Open the marvelous door.

  DOORTOUCH = false;
  triggerDoor(5, 2000);
  delay(100);
  triggerDoor(18, 2000);
  delay(100);
  triggerDoor(19, 2000);
  delay(100);
  triggerDoor(21, 2000);

  sData.origin = atticmaster;
  sData.sensor = attic_humanchain;
  sData.data = 111;
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sData, sizeof(sData));

  // delay(100);
  }
  else{
    if (millis() - timeout2 >= 2000)
    {
      timeout2 = millis();
      sData.origin = atticmaster;
      sData.sensor = attic_humanchain;
      sData.data = 0;
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sData, sizeof(sData));
    }
  }


//Emergency Escape Button
if ((digitalRead(23)) && (millis() - emergencyTrigger) >= 3000)
  {
    emergencyTrigger = millis();
    emergencyFlag = 0;
    Serial.println("OH NO HERE WE GO");
    sData.origin = 0x03;
    sData.sensor = 0xA3;
    sData.data = 1;
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sData, sizeof(sData));
    
    triggerDoor(5, 2000);
    triggerDoor(18, 2000);
    triggerDoor(19, 2000);
    triggerDoor(21, 2000);
    delay(50);
  }


//ESP32 GPIO Pin that doesn't appear to work reliably.
if(digitalRead(15)){
  DOORTOUCH = true;
  Serial.println("TRUE");
  delay(250);
}


  asynctimer.handle();
}