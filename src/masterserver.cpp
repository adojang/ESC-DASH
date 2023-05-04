/*
  --------------------------------------------------------------------------
  Escape Room Master Server
  Adriaan van Wijk
  22 May 2023

  This code is for a server which listens for messages from the remote ESP's
  and enables override functions on the remote ESP's to override puzzles.

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

#define NAME "escape"

/* Kernal*/
#include <Arduino.h>
#include <config.h>

/* ESP-DASH */
#include <ESPDashPro.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTimer.h>

/* Wifi and mDNS */
#include <WiFi.h>
#include <esp_wifi.h>
#include <ESPmDNS.h>

/* ESP-NOW */
#include <esp_now.h>

/* Elegant OTA */
#include <AsyncElegantOTA.h>
#include <ESPmDNS.h>


/* SET MAC ADDRESS */
uint8_t setMACAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x00}; // Address of Master Server
uint8_t testaddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xB0}; // Address of Master Server
// uint8_t MAC01[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x01};
// uint8_t MAC02[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x02};
// uint8_t MAC03[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x03};
// uint8_t MAC04[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x04};
// uint8_t MAC05[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x05};
// uint8_t MAC06[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x06};
// uint8_t MAC07[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x07};
// uint8_t MAC08[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x08};
// uint8_t MAC09[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA1};
// uint8_t MAC10[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA2};
// uint8_t MAC11[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xB0};

#define ADDRESSSLENGTH 11

uint8_t sensorAddress[][6] = {
    {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x01},
    {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x02},
    {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x03},
    {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x04},
    {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x05},
    {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x06},
    {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x07},
    {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x08},
    {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA1},
    {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA2},
    {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xB0}
};


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
 * 0xA1 - Keypad 1
 * 0xA2 - Keypad 2
 * 0xB0 - Relay Control
*/


/* ESP Async Timer */
AsyncTimer asynctimer;

/* WiFi Credentials */
const char* ssid = WIFI_SSID; // SSID
const char* password = WIFI_PASS; // Password

/* Setup */
AsyncWebServer server(80);
ESPDash dashboard(&server,false);
esp_now_peer_info_t peerInfo;

// ESPDash dashboard.setTitle("Escape Room Control Panel");
/* * * * * *  ESP-DASH Cards * * * * * * */

#define CARDLEN 8

Card cardArray[CARDLEN] = {
/* Attic */
Card(&dashboard, BUTTON_CARD, "Open Human Chain Door"), // momentary
Card(&dashboard, BUTTON_CARD, "Bicycle Lightbulb Override"), // not momentary
Card(&dashboard, BUTTON_CARD, "Clock Motor Override"), //momentary

/* Ancient Tomb */
Card(&dashboard, BUTTON_CARD, "Open Beetle Puzle"), //momentary
Card(&dashboard, BUTTON_CARD, "Open Chalice Door"), //momentary
Card(&dashboard, BUTTON_CARD, "Override Ring Reader"), //momentary
Card(&dashboard, BUTTON_CARD, "Tangrum Puzzle Override"), //momentary

/* All Aboard (Train) */
Card(&dashboard, BUTTON_CARD, "Open Thumb Reader Door"), //momentary
};

/* ESP-NOW Structures */
typedef struct dataPacket {
int trigger = 0;
} dataPacket;

dataPacket sData[CARDLEN];
dataPacket sDataprev[CARDLEN];

/* Data Order for cardArray and dataPacket
 * 0 - humanchain
 * 1 - bikelight
 * 2 - clockmotor
 * 3 - beetle
 * 4 - chalicedoor
 * 5 - ringreader
 * 6 - tangrumtomb
 * 7 - thumbreaderdoor
*/

/* Tabs */
Tab attic(&dashboard, "Attic");
Tab tomb(&dashboard, "Ancient Tomb");
Tab train(&dashboard, "All Aboard");

/* Timer Cards */

/* Overview Timer Cards */
Card overview_status(&dashboard, PROGRESS_CARD, "Time Remaining", "m", 0, 60);
Card overview_attic_time(&dashboard, PROGRESS_CARD, "Time Remaining", "m", 0, 60);
Card testnetwork(&dashboard, BUTTON_CARD, "TEST CONNECTION"); //momentary
Card overview_tomb_time(&dashboard, PROGRESS_CARD, "Time Remaining", "m", 0, 60);
Card overview_train_time(&dashboard, PROGRESS_CARD, "Time Remaining", "m", 0, 60);

/* Contained inside tabs*/
Card attic_time(&dashboard, PROGRESS_CARD, "Time Remaining", "m", 0, 60);
Card tomb_time(&dashboard, PROGRESS_CARD, "Time Remaining", "m", 0, 60);
Card train_time(&dashboard, PROGRESS_CARD, "Time Remaining", "m", 0, 60);

/* ESP-NOW */

/* ESP-NOW Structures */


// dataPacket sData; // data to send
dataPacket rData; // data to recieve

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");

}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  // memcpy(&rData, incomingData, sizeof(rData));
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

    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);
    // for (int i = 0; i < ADDRESSSLENGTH; i++) {
    //   memcpy(peerInfo.peer_addr, sensorAddress[i], 6); 
    //   peerInfo.channel = 0;  
    //   peerInfo.encrypt = false;    
    //   if (esp_now_add_peer(&peerInfo) != ESP_OK){
    //     Serial.println("Failed to add peer");
    //     return;
    //   }

      memcpy(peerInfo.peer_addr, testaddress, 6); 
      peerInfo.channel = 0;  
      peerInfo.encrypt = false;    
      if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
        return;
    //   }


    }


    

    // Register for a callback function that will be called when data is received

}



void testingnow(){
  sData->trigger=4;
  esp_err_t result = esp_now_send(testaddress, (uint8_t *) &sData, sizeof(sData));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }

}

void setup() {
  Serial.begin(115200);
  

 /* Setup Tabs */
  // dashboard.setAuthentication("admin", "1234"); // Authentication
  dashboard.setTitle("Escape Room Master Control");

  /* Attic */
  cardArray[0].setTab(&attic);
  cardArray[1].setTab(&attic);
  cardArray[2].setTab(&attic);
  attic_time.setTab(&attic);
  attic_time.setSize(6,6,6,6,6,6);

  /* Tomb */
  cardArray[3].setTab(&tomb);
  cardArray[4].setTab(&tomb);
  cardArray[5].setTab(&tomb);
  cardArray[6].setTab(&tomb);
  tomb_time.setTab(&tomb);
  tomb_time.setSize(6,6,6,6,6,6);
  /* Train */
  cardArray[7].setTab(&train);
  train_time.setTab(&train);
  train_time.setSize(6,6,6,6,6,6);

/* Connect WiFi */
  WiFi.mode(WIFI_STA);
  esp_wifi_set_mac(WIFI_IF_STA, &setMACAddress[0]);



  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.printf("WiFi Failed!\n");
      return;
  }
  //Local Only Mode

  // WiFi.mode(WIFI_AP);
  // WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
  // WiFi.softAP(ssid, NULL);

  Serial.println(WiFi.softAPIP());

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());


  if (!MDNS.begin("escape")) {
        Serial.println("Error setting up MDNS responder!");
        while(1) {
            delay(1000);
        }
    }
  Serial.println("mDNS responder started");

  /* Initialize Callback Functions */

  /* TEST CALLBACK FUNCTION*/

   testnetwork.attachCallback([](int value){
    testnetwork.update(value);
    Serial.printf("TEST BUTTON TRIGGERED: %d\n");
    sData->trigger=value;

    esp_err_t result = esp_now_send(testaddress, (uint8_t *) &sData, sizeof(sData));
   
    if (result == ESP_OK) { Serial.println("Sent with success");}
    else {Serial.println("Error sending the data");}
    
    dashboard.sendUpdates();
    });

  
    for (int i = 0; i < CARDLEN; i++){

    if (i != CARDLEN-1){

    
    cardArray[i].attachCallback([i](int value){
    sData[i].trigger = 1;
    cardArray[i].update(1);
    Serial.printf("Card triggered: %d\n", i);
    dashboard.sendUpdates();
    });

    }
    else
    {

  cardArray[i].attachCallback([i](int value){
    sData[i].trigger = 1;
    cardArray[i].update(1);
    Serial.printf("DOOR CARD!!! triggered: %d\n", i);
    testingnow();
     
    dashboard.sendUpdates();
    });

    }






  }

  /* Elegant OTA */
  AsyncElegantOTA.begin(&server, "admin", "admin1234");

  server.begin();
  MDNS.addService("http", "tcp", 80);

  startespnow();



  //Begin Sending Data to Remote ESP's every 250ms

    // asynctimer.setInterval([]() { testingnow();},  3000);

}

void setButtonFalse(int i){

    sData[i].trigger = 0;
    cardArray[i].update(0);
    dashboard.sendUpdates();
    Serial.printf("Card %d, disabled\n", i);
}

void loop() {
//Detect and Handle Status Reset when a button is pressed.
  for (int i=0; i < CARDLEN; i++)
  {
    if (sData[i].trigger != sDataprev[i].trigger)
    {
      Serial.println("Timer Triggered 4s");


    asynctimer.setTimeout([i]() {
      setButtonFalse(i);
    }, 2000);
    }
  }



  //Reset Check if Card Triggered
  for (int i; i < CARDLEN; i++){
    sDataprev[i] = sData[i];
  }





  asynctimer.handle();
  // dashboard.sendUpdates();
}