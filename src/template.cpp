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

#define NAME "template"
#define MACAD 0xEE // Refer to Table in Conventions

/* Data Naming Convention for Mac Addresses

*  0x00 - masterserver


 * 0xA0 - humanchain
 * 0xA1 - bikelight
 * 0xA2 - clockmotor

 * 0xB0 - beetle
 * 0xB1 - chalicessensor
 * 0xB2 - ringreader
 * 0xB3 - tangrumtomb

 * 0xC0 - thumbreader
 * 0xC1 - Keypad 1
 * 0xC2 - Keypad 2

 * 0xD0 - relaycontrol

 * 0xEE - template

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

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x00}; // Address of Master Server
uint8_t setMACAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, MACAD};


/* ESP Async Timer */
AsyncTimer asynctimer(20);

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


bool turnlighton = false;


/* Example Function on how to send data to another ESP that you can remove*/

void sendData()
{
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sData, sizeof(sData));
      if (result == ESP_OK) { Serial.println("Sent with success");}
      else {Serial.println("Error sending the data");}

}

/* ESP-NOW Callback Functions*/

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&rData, incomingData, sizeof(rData));
  Serial.println("Override Data Recieved...");

  if (rData.trigger == 1) {
  //You should never use delay in this function. It might cause the ESP-NOW to crash.
    turnlighton = true;
  }
  else
  {
    turnlighton = false;
  }
  
  // Add your code here to do something with the data recieved.
  //It's probably best to use a flag instead of calling it directly here. Not Sure

  //Demonstration Sending Data:

  sendData();


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

}


int globalwait = 0;
int normalDelay = 650;
int longDelay = normalDelay * 2;
int morsebootflag = 0;
int morseperiod = 0;
unsigned long morsetimer = 0;

void shortlight() {
  turnlighton = true;
  asynctimer.setTimeout([]() {
    turnlighton = false;
    Serial.println("Light Off");
  }, normalDelay);
}

void longlight(){ // turn light on, then wait for delay to turn off again.
  turnlighton = true;

  asynctimer.setTimeout([]() {
    turnlighton = false;
    Serial.println("Light Off");
  }, longDelay);
}

void breifpause(){
  globalwait += longDelay;
}

void endloop(){

morseperiod = (millis()-morsetimer) + (2*longDelay);
morsebootflag = 1;
Serial.println("Morse Period:");
Serial.println(morseperiod);
}

void dot(){
  asynctimer.setTimeout([]() {shortlight(); }, globalwait);
    globalwait += normalDelay*2;
    Serial.printf("New Wait Time: %d\n", globalwait);
    }

    void dash(){
  asynctimer.setTimeout([]() {longlight(); }, globalwait);
    globalwait += longDelay*2;
    Serial.printf("New Wait Time: %d\n", globalwait);
    }

void morseloop(){
morsetimer = millis();
globalwait = 0;

dot();
dot();
dot();
dash();
dash();
breifpause();

dot();
dot();
dot();
dot();
dot();
breifpause();

dot();
dash();
dash();
dash();
dash();

if (morsebootflag == 0){
  asynctimer.setTimeout([]() {endloop();}, globalwait); // wait for 6200 before executing
}

Serial.println("Morse End");
}


void setup() {
  Serial.begin(115200);
  // startwifi();
  // startespnow();
  //Make any Edits you need to add below this line ------------------------------

  pinMode(2, OUTPUT);
  digitalWrite(2,LOW);

    ledcSetup(0, 100, 12); // Lightbulb
  ledcSetup(1, 100, 12); // Bike LED
  ledcAttachPin(27, 0); // Lightbulb
  ledcAttachPin(26, 1);
  ledcWrite(0, 0);
  ledcWrite(1, 0);


    //Initialize and calculate the total period time.

      //Initialize
      morseloop();
     //CRASHES BECAUSE I DONT HAE ENOUGH TIMERS AVAILABLE



  //Here are two asynchronus timers you can use to run functions.
  //See https://github.com/Aasim-A/AsyncTimer
  // For documentations

  //  asynctimer.setInterval([]() {esp_now_send(broadcastAddress, (uint8_t *) &sData, sizeof(sData));},  5000);
  // asynctimer.setTimeout([]() {Serial.println("Hello world!");}, 2000);
// "Hello world!" will be printed to the Serial once after 2 seconds
}



void loop() {
  if(morsebootflag == 1)
  {
    Serial.println("morsebootflag = 1");
      asynctimer.setInterval([]() {
      Serial.println("Set Interval");
      morseloop();
    }, morseperiod);
    morsebootflag = 2;
  }


  if (turnlighton) {
    ledcWrite(0, 4000);
    digitalWrite(2,HIGH);
  }
  else {
    ledcWrite(0, 0);
    digitalWrite(2,LOW);
  }



  //Required for the asynctimer to work.
  asynctimer.handle();
}
