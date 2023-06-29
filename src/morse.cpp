/*
  --------------------------------------------------------------------------
  This Morse code doesn't use wifi because of the bad signal.
  Adriaan van Wijk
  22 May 2023

  12 Key Keypad that opens an electromagnetic lock when the correct code is entered.

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

#define NAME "morse"
#define MACAD 0xA9 // Manually Refer to Table in Conventions

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
uint8_t broadcastAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x03}; // Address of Attic Master
uint8_t setMACAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, MACAD};

/* ESP Async Timer */
AsyncTimer asynctimer(15);

/* WiFi Credentials */
const char *ssid = WIFI_SSID;     // SSID
const char *password = WIFI_PASS; // Password

/* ESP-NOW Structures */

dataPacket sData; // data to send
dataPacket rData; // data to recieve

/* Setup */
AsyncWebServer server(80);
esp_now_peer_info_t peerInfo;




/* ESP-NOW Callback Functions*/
int resend = 0;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");
  if (status != ESP_NOW_SEND_SUCCESS){
    resend = 1;
    
  }
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    memcpy(&rData, incomingData, sizeof(rData));
    Serial.println("Data Recieved from Somewhere");
    Serial.print("Data Origin: ");

}



void startwifi(){

  // Set device as a Wi-Fi Station
  WiFi.softAP(NAME, "pinecones", 0, 1, 4);
  WiFi.mode(WIFI_AP_STA);
  esp_wifi_set_mac(WIFI_IF_STA, &setMACAddress[0]);
  unsigned long wifitimeout = millis();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
    if ((millis() - wifitimeout) > 10000) ESP.restart();
  }
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.printf("WiFi Failed!\n");
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

void startespnow()
{
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register Callback Functions
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
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
unsigned short IDx; // ID for morse loop.
int resetmorseflag = 0;


void shortlight() {
  ledcWrite(0, 4000);
  asynctimer.setTimeout([]() {
    ledcWrite(0, 0);
    Serial.println("Light Off");
  }, normalDelay);
}

void longlight(){ // turn light on, then wait for delay to turn off again.
  ledcWrite(0, 4000);

  asynctimer.setTimeout([]() {
    ledcWrite(0, 0);
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
dot(); // delay slightly longer.
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


void setup()
{
  Serial.begin(115200);
 // Make any Edits you need to add below this line ------------------------------
  ledcSetup(0, 100, 12); // Lightbulb
  ledcAttachPin(27, 0); // Lightbulb
  ledcWrite(0, 4000);

  // startwifi();
  // startespnow();

  // Make any Edits you need to add below this line ------------------------------

  //Morse Code Initialize:
  morseloop(); // Run once to find the amount of time it takes.
}


void loop()
{

 if(morsebootflag == 1)
  {
    Serial.println("morsebootflag = 1"); // end of loop and initial run.
      IDx = asynctimer.setInterval([]() {
      Serial.println("Set Interval");
      morseloop();
    }, morseperiod);
    morsebootflag = 2;
  }

  // Required for the asynctimer to work.
  asynctimer.handle();
}