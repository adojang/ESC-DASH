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
#include "encode.h"
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



#define ADDRESSSLENGTH 11

/* SET MAC ADDRESS */
uint8_t setMACAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x00}; // Address of Master Server

// Control
uint8_t m_trainmaster[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x01};
uint8_t m_tombmaster[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x02};
uint8_t m_atticmaster[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x03};

// Attic
uint8_t m_attic_humanchain[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA0};
uint8_t m_attic_bike[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA1};
uint8_t m_attic_grandfatherclock[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA2};
uint8_t m_attic_overrideButton[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA3};

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

// Template
uint8_t m_temp_TEMPLATE[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xEE};

// RFID Readers

// uint8_t m_RFID1[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, attic_RFID1};
// uint8_t m_RFID2[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, attic_RFID2};
// uint8_t m_RFID3[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA9};
// uint8_t m_RFID4[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, attic_RFID4};


// EMERGENCY
unsigned long emergencyTrigger = 0;

/* ESP Async Timer */
AsyncTimer asynctimer(15);

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

/*Testing*/
Card touchval(&dashboard, GENERIC_CARD, "If these numbers stop changing, restart everything.");
Card restart_attic(&dashboard, BUTTON_CARD, "Reset Attic"); //momentary
Card restart_master(&dashboard, BUTTON_CARD, "Restart Control"); //momentary
Card DOORTOUCH(&dashboard, GENERIC_CARD, "DoorTouch"); //momentary
// Card sennet_card(&dashboard, BUTTON_CARD, "Override Sennet Puzzle"); //momentary


/* Attic */
Card humanchain_card(&dashboard, BUTTON_CARD, "Open Human Chain Door"); // momentary
Card bike_card(&dashboard, BUTTON_CARD, "Bicycle Lightbulb Override"); // not momentary
Card grandfatherclock_card(&dashboard, BUTTON_CARD, "Clock Motor Override"); //momentary
Card bike_speed(&dashboard, GENERIC_CARD, "Bike Speed"); //momentary


/* Ancient Tomb */
Card sennet_card(&dashboard, BUTTON_CARD, "Override Sennet Puzzle"); //momentary
Card chalice_card(&dashboard, BUTTON_CARD, "Open Chalice Door"); //momentary
Card ringreader_card(&dashboard, BUTTON_CARD, "Override Ring Reader"); //momentary
Card tangrumtomb_card(&dashboard, BUTTON_CARD, "Open Tomb"); //momentary

/* All Aboard (Train) */
Card trainroomdoor_card(&dashboard, BUTTON_CARD, "Open Train Room Door"); //momentary
Card thumbreader_card(&dashboard, BUTTON_CARD, "Overide Thumb Reader"); //momentary


/* ESP-NOW Structures */

dataPacket sData;
dataPacket sDataprev;
dataPacket rData; // data to recieve


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

/* Overview Timer Cards */
// Card testnetwork(&dashboard, BUTTON_CARD, "TEST CONNECTION"); //momentary

    /* Status of Masters */
Card train_status(&dashboard, STATUS_CARD, "Train  Status", "success");
Card tomb_status(&dashboard, STATUS_CARD, "Tomb Status", "success");
Card attic_status(&dashboard, STATUS_CARD, "Attic Status", "success");

Card attic_rfid1(&dashboard, GENERIC_CARD, "RFID1 Status (Not Working Yet, Bypassed)");
Card attic_rfid2(&dashboard, GENERIC_CARD, "RFID2 Status");
Card attic_rfid3(&dashboard, GENERIC_CARD, "RFID3 Status");
Card attic_rfid4(&dashboard, GENERIC_CARD, "RFID4 Status");


/* Contained inside tabs*/
// Card attic_time(&dashboard, PROGRESS_CARD, "Time Remaining", "m", 0, 60);
// Card tomb_time(&dashboard, PROGRESS_CARD, "Time Remaining", "m", 0, 60);
// Card train_time(&dashboard, PROGRESS_CARD, "Time Remaining", "m", 0, 60);

int keypadtrigger = 0;

//Global Status
// int global_status;


void triggerDoor(int pin){
  digitalWrite(pin, LOW);
  Serial.println("Door Opened");
  sData.origin = masterserver;
  sData.sensor = masterserver;
  sData.data = 1; // Door Open
  esp_err_t result = esp_now_send(m_train_keypad, (uint8_t *) &sData, sizeof(sData));
  delay(50);
  asynctimer.setTimeout([pin]() {
      digitalWrite(pin, HIGH);
      Serial.println("Door Closed");
      sData.origin = masterserver;
      sData.sensor = masterserver;
      sData.data = 0; // Door closed
      esp_err_t result = esp_now_send(m_train_keypad, (uint8_t *) &sData, sizeof(sData));
      delay(50);
    }, 3000);
}



// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");

  // if (status == ESP_NOW_SEND_SUCCESS){
  //   global_status = 1;
  // }
  // else
  // {
  //   global_status = 0;
  // }

}

unsigned short attictimeout = 999;
unsigned short traintimeout = 999;
unsigned short tombtimout = 999;


// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    memcpy(&rData, incomingData, sizeof(rData));
    // Serial.println("Data Recieved from Somewhere");
    // Serial.print("Data Origin: ");
    // Serial.pr/intln(rData.origin);

    if(rData.origin == train_keypad && rData.sensor == train_keypad){
    keypadtrigger = rData.data;

    }

    if(rData.origin == attic_humanchain && rData.sensor == attic_humanchain){

      touchval.update(rData.data);
      dashboard.sendUpdates();
    }

    if(rData.origin == atticmaster && rData.sensor == attic_humanchain)
    {
      DOORTOUCH.update(rData.data);
      dashboard.sendUpdates();
    }


    if(rData.origin == attic_RFID1 && rData.sensor == attic_RFID1)
    {
      attic_rfid1.update(rData.data);
      if (rData.data == 100) attic_rfid1.update("COMPLETE");
      dashboard.sendUpdates();
    }

    if(rData.origin == attic_RFID2 && rData.sensor == attic_RFID2)
    {
      attic_rfid2.update(rData.data);
      if (rData.data == 100) attic_rfid2.update("COMPLETE");
      dashboard.sendUpdates();
    }

    if(rData.origin == attic_RFID3 && rData.sensor == attic_RFID3)
    {
      attic_rfid3.update(rData.data);
    if (rData.data == 100) attic_rfid3.update("COMPLETE");
      dashboard.sendUpdates();
    }

    if(rData.origin == attic_RFID4 && rData.sensor == attic_RFID4)
    {
      attic_rfid4.update(rData.data);
      if (rData.data == 100) attic_rfid4.update("COMPLETE");
      dashboard.sendUpdates();
    }

       

  

   if(rData.origin == attic_bike && rData.sensor == attic_bike)
    {
    bike_speed.update(rData.data);
    dashboard.sendUpdates();
    }


        if(rData.origin == atticmaster && rData.sensor == atticmaster && rData.data == 0)
    {
 //Set Status as Alive:
    }
   

 
  }



// END STATUS CHECK

void startespnow(){
    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
      return;
    }

    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);

    /* Register All 3 Master Peers*/

    /* Attic */
    memcpy(peerInfo.peer_addr, m_atticmaster, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
      Serial.println("Failed to add peer");
      return;
    }

    /* Tomb */
    memcpy(peerInfo.peer_addr, m_tombmaster, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
      Serial.println("Failed to add peer");
      return;
    }

    /* Train */

    memcpy(peerInfo.peer_addr, m_train_keypad, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
      Serial.println("Failed to add peer");
      return;
    }
}

void configDash(){
  /* Configure ESP-Dash */
  /* Setup Tabs */
  // dashboard.setAuthentication("admin", "1234"); // Authentication
  dashboard.setTitle("Escape Room Master Control");

  sData.origin = 0x00;

  /* Overview */

    attic_status.setSize(6,6,6,6,6,6);
    train_status.setSize(6,6,6,6,6,6);
    tomb_status.setSize(6,6,6,6,6,6);
  /* Status of Masters */



  /* Attic */
  humanchain_card.setTab(&attic);
  bike_card.setTab(&attic);
  grandfatherclock_card.setTab(&attic);
  humanchain_card.setSize(6,6,6,6,6,6);
  bike_card.setSize(6,6,6,6,6,6);
  grandfatherclock_card.setSize(6,6,6,6,6,6);


  /* Tomb */
  sennet_card.setTab(&tomb);
  chalice_card.setTab(&tomb);
  ringreader_card.setTab(&tomb);
  tangrumtomb_card.setTab(&tomb);
  sennet_card.setSize(6,6,6,6,6,6);
  chalice_card.setSize(6,6,6,6,6,6);
  tangrumtomb_card.setSize(6,6,6,6,6,6);
  ringreader_card.setSize(6,6,6,6,6,6);
  /* Train */
  thumbreader_card.setTab(&train);
  trainroomdoor_card.setTab(&train);
  thumbreader_card.setSize(6,6,6,6,6,6);
  trainroomdoor_card.setSize(6,6,6,6,6,6);




}

void buttonTimeout(Card* cardptr, int timeout = 3000){

  cardptr->update(1);
  asynctimer.setTimeout([cardptr]() {
    cardptr->update(0);
    dashboard.sendUpdates();
  }, timeout);

  dashboard.sendUpdates();
  

}

void startButtonCB(){


/* Reset Button Attic */
restart_attic.attachCallback([](int value){
buttonTimeout(&restart_attic);
Serial.printf("Attic Restart Triggered\n");
sData.origin = masterserver;
sData.data = 66;
esp_err_t result = esp_now_send(m_atticmaster, (uint8_t *) &sData, sizeof(sData));
if (result != ESP_OK) { Serial.println("ERROR SENDING ESP-NOW DATA");}

sData.data = 0;






});




/* Restart This Master Server*/

restart_master.attachCallback([](int value){
buttonTimeout(&restart_master);
Serial.printf("Master Restart Triggered\n");
ESP.restart();

});


/* 0 - HumanChainDoor */
humanchain_card.attachCallback([](int value){
// humanchain_card.update(1);
buttonTimeout(&humanchain_card);
Serial.printf("Attic Door Triggered\n");
sData.sensor = attic_humanchain; 
sData.data = 1;
esp_err_t result = esp_now_send(m_atticmaster, (uint8_t *) &sData, sizeof(sData));
if (result != ESP_OK) { Serial.println("ERROR SENDING ESP-NOW DATA");}
sData.data = 0;
  
});

/* 0xA1 - bikelight - THIS PUZZLE IS NOT MOMENTARY */
bike_card.attachCallback([](int value){
bike_card.update(value);

     
        sData.origin = masterserver;
        sData.sensor = attic_humanchain;
        sData.data = 1;
      esp_err_t result = esp_now_send(m_atticmaster, (uint8_t *) &sData, sizeof(sData));

// Serial.printf("Bike Light Enabled\n");
// sData.origin = masterserver;
// sData.sensor = attic_bike;
// sData.data = 1;
// esp_err_t result = esp_now_send(m_atticmaster, (uint8_t *) &sData, sizeof(sData));
// if (result != ESP_OK) { Serial.println("ERROR SENDING ESP-NOW DATA");}
// sData.data = 0;
// dashboard.sendUpdates();
});

/* 0xA2 - grandfatherclock */
grandfatherclock_card.attachCallback([](int value){
buttonTimeout(&grandfatherclock_card);
Serial.printf("Grandfather Clock Triggered\n");
sData.sensor = attic_grandfatherclock; 
sData.data = 1;
esp_err_t result = esp_now_send(m_attic_grandfatherclock, (uint8_t *) &sData, sizeof(sData));
if (result != ESP_OK) { Serial.println("ERROR SENDING ESP-NOW DATA");}
sData.data = 0;

});

/* 0xB0 - sennet table */
sennet_card.attachCallback([](int value){
buttonTimeout(&sennet_card);
Serial.printf("Sennet Table Triggered\n");
sData.sensor = tomb_sennet; 
sData.data = 1;
esp_err_t result = esp_now_send(m_tomb_sennet, (uint8_t *) &sData, sizeof(sData));
if (result != ESP_OK) { Serial.println("ERROR SENDING ESP-NOW DATA");}
sData.data = 0;

});

/* 0xB1 - chalicessensor */
chalice_card.attachCallback([](int value){
buttonTimeout(&chalice_card);
Serial.printf("Chalice Sensor Triggered\n");
sData.sensor = tomb_chalice; 
sData.data = 1;
esp_err_t result = esp_now_send(m_tomb_chalice, (uint8_t *) &sData, sizeof(sData));
if (result != ESP_OK) { Serial.println("ERROR SENDING ESP-NOW DATA");}
sData.data = 0;

});

/* 0xB2 - ringreader */
ringreader_card.attachCallback([](int value){
buttonTimeout(&ringreader_card);
Serial.printf("Ring Reader Triggered\n");
sData.sensor = tomb_ringReader; 
sData.data = 1;
esp_err_t result = esp_now_send(m_tomb_ringReader, (uint8_t *) &sData, sizeof(sData));
if (result != ESP_OK) { Serial.println("ERROR SENDING ESP-NOW DATA");}
sData.data = 0;

});

/* 0xB3 - tangrumtomb */
tangrumtomb_card.attachCallback([](int value){
buttonTimeout(&tangrumtomb_card);
Serial.printf("Tangram Tomb Triggered\n");
sData.sensor = tomb_tangrum; 
sData.data = 1;
esp_err_t result = esp_now_send(m_tomb_tangrum, (uint8_t *) &sData, sizeof(sData));
if (result != ESP_OK) { Serial.println("ERROR SENDING ESP-NOW DATA");}
sData.data = 0;

});
/* 0xC0 - Train Room */
  trainroomdoor_card.attachCallback([](int value){
  buttonTimeout(&trainroomdoor_card);
  Serial.printf("Train Room Triggered\n");
sData.sensor = train_keypad; 
sData.data = 1;
  esp_err_t result = esp_now_send(m_trainmaster, (uint8_t *) &sData, sizeof(sData));
  if (result != ESP_OK) { Serial.println("ERROR SENDING ESP-NOW DATA");}
  sData.data = 0;
  
});


/* 0xC1 - thumbreader */
thumbreader_card.attachCallback([](int value){
buttonTimeout(&thumbreader_card);
  Serial.printf("Thumbreader Triggered\n");
sData.sensor = train_thumb; 
sData.data = 1;
  esp_err_t result = esp_now_send(m_trainmaster, (uint8_t *) &sData, sizeof(sData));
  if (result != ESP_OK) { Serial.println("ERROR SENDING ESP-NOW DATA");}
  sData.data = 0;

});




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

  if (!MDNS.begin("escape"))
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

  server.begin();
  MDNS.addService("http", "tcp", 80);

}


// }


void setup() {
  Serial.begin(115200);
  //Signaling LED
    pinMode(2, OUTPUT);
  digitalWrite(2,LOW);

  
  //Emergency Buttons.
  pinMode(25,OUTPUT);
  digitalWrite(25,HIGH);
  pinMode(4,INPUT_PULLDOWN);

// Enable relay. This might be an issue. DANGER LINE DANGER LINE DANGINER LINE DANGER LINE DANGER LINE DANGINER LINE   
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
  
// DANGER LINE DANGER LINE DANGINER LINE DANGER LINE DANGER LINE DANGINER LINE DANGER LINE DANGER LINE DANGINER LINE

  /* Start Wifi and ESP-DASH*/
  startWifi();
  configDash();
  startButtonCB();
  startespnow();

  //Start Checking Server Status every 5s
    //  asynctimer.setInterval([]() {
    //   serverstatus();
    // }, 5000);

  emergencyTrigger = millis();
}


void loop() {

delay(25); // This seems to solve all my self-triggering issues. Probably due to the pins being super sensitive.


//Emergency Escape Button
if ((digitalRead(4)) && ((millis() - emergencyTrigger) >= 5000))
  {
  emergencyTrigger = millis();
  Serial.println("OH NO HERE WE GO");
  
  triggerDoor(5);

  delay(50);
}

if (keypadtrigger == 1) // Unlock the Door Steady State
  {
  keypadtrigger = 0;
  digitalWrite(5, LOW);
  delay(50);
}

if (keypadtrigger == 2) // Lock the Door Steady State
  {
  keypadtrigger = 0;
  digitalWrite(5, HIGH);
  delay(50);
}

  asynctimer.handle();
  // dashboard.sendUpdates();
}