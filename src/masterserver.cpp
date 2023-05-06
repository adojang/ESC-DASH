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
uint8_t templateaddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xEE}; // Address of Template

#define ADDRESSSLENGTH 11

uint8_t humanchain[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA0};
uint8_t bike[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA1};
uint8_t grandfatherclock[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA2};
uint8_t beetle[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xB0};
uint8_t chalicessensor[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xB1};
uint8_t ringreader[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xB2};
uint8_t tangrumtomb[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xB3};
uint8_t thumbreader[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xC0};
uint8_t keypad1[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xC1};
uint8_t keypad2[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xC2};
uint8_t relaycontrol[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xD0};


// uint8_t sensorAddress[][6] = {
//     {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA0},
//     {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA1},
//     {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA2},
//     {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xB0},
//     {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xB1},
//     {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xB2},
//     {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xB3},
//     {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xC0},
//     {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xC1},
//     {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xC2},
//     {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xD0}
// };

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
 * 
 * 0xEE - template

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


/* Attic */
Card humanchain_card(&dashboard, BUTTON_CARD, "Open Human Chain Door"); // momentary
Card bike_card(&dashboard, BUTTON_CARD, "Bicycle Lightbulb Override"); // not momentary
Card grandfatherclock_card(&dashboard, BUTTON_CARD, "Clock Motor Override"); //momentary

/* Ancient Tomb */
Card beetle_card(&dashboard, BUTTON_CARD, "Open Beetle Puzle"); //momentary
Card chalice_card(&dashboard, BUTTON_CARD, "Open Chalice Door"); //momentary
Card ringreader_card(&dashboard, BUTTON_CARD, "Override Ring Reader"); //momentary
Card tangrumtomb_card(&dashboard, BUTTON_CARD, "Tangrum Puzzle Override"); //momentary

/* All Aboard (Train) */
Card trainroomdoor_card(&dashboard, BUTTON_CARD, "Open Train Room Door"); //momentary
Card thumbreader_card(&dashboard, BUTTON_CARD, "Open Thumb Reader Door"); //momentary


/* ESP-NOW Structures */
typedef struct dataPacket {
int trigger = 0;
} dataPacket;

dataPacket sData;
dataPacket sDataprev;

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
  Serial.println("Data Recieved from Somewhere lol.");

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

    /* Register All the Peers*/

      memcpy(peerInfo.peer_addr, templateaddress, 6); 
      peerInfo.channel = 0;  
      peerInfo.encrypt = false;    
      if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
        return;
    }

          memcpy(peerInfo.peer_addr, humanchain, 6); 
      peerInfo.channel = 0;  
      peerInfo.encrypt = false;    
      if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
        return;
    }

          memcpy(peerInfo.peer_addr, bike, 6); 
      peerInfo.channel = 0;  
      peerInfo.encrypt = false;    
      if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
        return;
    }

          memcpy(peerInfo.peer_addr, grandfatherclock, 6); 
      peerInfo.channel = 0;  
      peerInfo.encrypt = false;    
      if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
        return;
    }

          memcpy(peerInfo.peer_addr, beetle, 6); 
      peerInfo.channel = 0;  
      peerInfo.encrypt = false;    
      if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
        return;
    }

          memcpy(peerInfo.peer_addr, chalicessensor, 6); 
      peerInfo.channel = 0;  
      peerInfo.encrypt = false;    
      if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
        return;
    }

          memcpy(peerInfo.peer_addr, ringreader, 6); 
      peerInfo.channel = 0;  
      peerInfo.encrypt = false;    
      if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
        return;
    }

          memcpy(peerInfo.peer_addr, tangrumtomb, 6); 
      peerInfo.channel = 0;  
      peerInfo.encrypt = false;    
      if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
        return;
    }
          memcpy(peerInfo.peer_addr, thumbreader, 6); 
      peerInfo.channel = 0;  
      peerInfo.encrypt = false;    
      if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
        return;
    }
          memcpy(peerInfo.peer_addr, keypad1, 6); 
      peerInfo.channel = 0;  
      peerInfo.encrypt = false;    
      if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
        return;
    }

          memcpy(peerInfo.peer_addr, keypad2, 6); 
      peerInfo.channel = 0;  
      peerInfo.encrypt = false;    
      if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
        return;
    }
          memcpy(peerInfo.peer_addr, relaycontrol, 6); 
      peerInfo.channel = 0;  
      peerInfo.encrypt = false;    
      if (esp_now_add_peer(&peerInfo) != ESP_OK){
      Serial.println("Failed to add peer");
      return;
    }

}





void setup() {
  Serial.begin(115200);
  

 /* Setup Tabs */
  // dashboard.setAuthentication("admin", "1234"); // Authentication
  dashboard.setTitle("Escape Room Master Control");

  /* Attic */
  humanchain_card.setTab(&attic);
  bike_card.setTab(&attic);
  grandfatherclock_card.setTab(&attic);
  attic_time.setTab(&attic);
  attic_time.setSize(6,6,6,6,6,6);

  /* Tomb */
  beetle_card.setTab(&tomb);
  chalice_card.setTab(&tomb);
  ringreader_card.setTab(&tomb);
  tangrumtomb_card.setTab(&tomb);
  tomb_time.setTab(&tomb);
  tomb_time.setSize(6,6,6,6,6,6);
  /* Train */
  thumbreader_card.setTab(&train);
  train_time.setTab(&train);
  train_time.setSize(6,6,6,6,6,6);

/* Connect WiFi */
 WiFi.softAP(NAME, "pinecones", 0, 1, 4);
 WiFi.mode(WIFI_AP_STA);
  esp_wifi_set_mac(WIFI_IF_STA, &setMACAddress[0]);

  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("WiFi Failed!\n");
    return;
  }

  if (!MDNS.begin("escape"))
  {
    Serial.println("Error setting up MDNS responder!");
    while (1)
    {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  /* Initialize Callback Functions */

  /* TEST CALLBACK FUNCTION FOR BLUE LED BUTTON*/

  testnetwork.attachCallback([](int value){
  testnetwork.update(value);
  Serial.printf("TEST BUTTON TRIGGERED: %d\n");
  sData.trigger=value;

  esp_err_t result = esp_now_send(templateaddress, (uint8_t *) &sData, sizeof(sData));
  
  if (result == ESP_OK) { Serial.println("Sent with success");}
  else {Serial.println("Error sending the data");}
  
  dashboard.sendUpdates();
  });

  /* These are the callbacks for all the buttons */


  /* 0 - HumanChainDoor */
  humanchain_card.attachCallback([](int value){
  humanchain_card.update(1);
  Serial.printf("Attic Door Triggered\n");
  sData.trigger = 4;
  esp_err_t result = esp_now_send(relaycontrol, (uint8_t *) &sData, sizeof(sData));
  if (result != ESP_OK) { Serial.println("ERROR SENDING ESP-NOW DATA");}
  sData.trigger = 0;
    
  dashboard.sendUpdates();
  });

/* 0xA1 - bikelight */
bike_card.attachCallback([](int value){
bike_card.update(value);
Serial.printf("Bike Light Triggered\n");
sData.trigger = value;
esp_err_t result = esp_now_send(bike, (uint8_t *) &sData, sizeof(sData));
if (result != ESP_OK) { Serial.println("ERROR SENDING ESP-NOW DATA");}
sData.trigger = 0;
dashboard.sendUpdates();
});

/* 0xA2 - grandfatherclock */
grandfatherclock_card.attachCallback([](int value){
grandfatherclock_card.update(1);
Serial.printf("Grandfather Clock Triggered\n");
sData.trigger = 1;
esp_err_t result = esp_now_send(grandfatherclock, (uint8_t *) &sData, sizeof(sData));
if (result != ESP_OK) { Serial.println("ERROR SENDING ESP-NOW DATA");}
sData.trigger = 0;
dashboard.sendUpdates();
});

/* 0xB0 - beetle */
beetle_card.attachCallback([](int value){
beetle_card.update(1);
Serial.printf("Beetle Triggered\n");
sData.trigger = 1;
esp_err_t result = esp_now_send(beetle, (uint8_t *) &sData, sizeof(sData));
if (result != ESP_OK) { Serial.println("ERROR SENDING ESP-NOW DATA");}
sData.trigger = 0;
dashboard.sendUpdates();
});

/* 0xB1 - chalicessensor */
chalice_card.attachCallback([](int value){
chalice_card.update(1);
Serial.printf("Chalice Sensor Triggered\n");
sData.trigger = 2;
esp_err_t result = esp_now_send(relaycontrol, (uint8_t *) &sData, sizeof(sData));
if (result != ESP_OK) { Serial.println("ERROR SENDING ESP-NOW DATA");}
sData.trigger = 0;
dashboard.sendUpdates();
});

/* 0xB2 - ringreader */
ringreader_card.attachCallback([](int value){
ringreader_card.update(1);
Serial.printf("Ring Reader Triggered\n");
sData.trigger = 1;
esp_err_t result = esp_now_send(ringreader, (uint8_t *) &sData, sizeof(sData));
if (result != ESP_OK) { Serial.println("ERROR SENDING ESP-NOW DATA");}
sData.trigger = 0;
dashboard.sendUpdates();
});

/* 0xB3 - tangrumtomb */
tangrumtomb_card.attachCallback([](int value){
tangrumtomb_card.update(1);
Serial.printf("Tangram Tomb Triggered\n");
sData.trigger = 3;
esp_err_t result = esp_now_send(relaycontrol, (uint8_t *) &sData, sizeof(sData));
if (result != ESP_OK) { Serial.println("ERROR SENDING ESP-NOW DATA");}
sData.trigger = 0;
dashboard.sendUpdates();
});

/* 0xC0 - thumbreader */
thumbreader_card.attachCallback([](int value){
  thumbreader_card.update(1);
  Serial.printf("Thumbreader Triggered\n");
  sData.trigger = 1;
  esp_err_t result = esp_now_send(thumbreader, (uint8_t *) &sData, sizeof(sData));
  if (result != ESP_OK) { Serial.println("ERROR SENDING ESP-NOW DATA");}
  sData.trigger = 0;
  
  dashboard.sendUpdates();
});

/* 0xC1 and 0xC2 - Train Room */
  trainroomdoor_card.attachCallback([](int value){
  trainroomdoor_card.update(1);
  Serial.printf("Train Room Triggered\n");
  sData.trigger = 1;
  esp_err_t result = esp_now_send(relaycontrol, (uint8_t *) &sData, sizeof(sData));
  if (result != ESP_OK) { Serial.println("ERROR SENDING ESP-NOW DATA");}
  sData.trigger = 0;
  
  dashboard.sendUpdates();
});







  /* Elegant OTA */
  AsyncElegantOTA.begin(&server, "admin", "admin1234");

  server.begin();
  MDNS.addService("http", "tcp", 80);



  startespnow();



  //Begin Sending Data to Remote ESP's every 250ms

    // asynctimer.setInterval([]() { testingnow();},  3000);

}

// void setButtonFalse(int i){

//     sData.trigger = 0;
//     cardArray[i].update(0);
//     dashboard.sendUpdates();
//     Serial.printf("Card %d, disabled\n", i);
// }

void loop() {
    //This line is sort of required. It automatically sends the data every 5 seconds. Don't know why. But hey there it is.
  // asynctimer.setInterval([]() {esp_now_send(templateaddress, (uint8_t *) &sData, sizeof(sData));},  5000);
//Detect and Handle Status Reset when a button is pressed.
  // for (int i=0; i < CARDLEN; i++)
  // {
  //   if (sData[i].trigger != 0)
  //   {
  //     sData[i].trigger = 0;
  //     Serial.println("Timer Triggered 2s");


  //   asynctimer.setTimeout([i]() {
  //     setButtonFalse(i);
  //   }, 2000);
  //   }
  // }



  // //Reset Check if Card Triggered
  // for (int i; i < CARDLEN; i++){
  //   sDataprev[i] = sData[i];
  // }





  asynctimer.handle();
  // dashboard.sendUpdates();
}