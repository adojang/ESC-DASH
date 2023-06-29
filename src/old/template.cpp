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

//https://docs.espdash.pro/cards/button/

#define NAME "trees"

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

#define ADDRESSSLENGTH 11

/* SET MAC ADDRESS */
uint8_t setMACAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x00}; // Address of Master Server

/* ESP Async Timer */
AsyncTimer asynctimer(15);

/* WiFi Credentials */
const char* ssid = "Mourning to Dancing"; // SSID
const char* password = "throughchristalone"; // Password

/* Setup */
AsyncWebServer server(80);
ESPDash dashboard(&server,false);
esp_now_peer_info_t peerInfo;

// ESPDash dashboard.setTitle("Escape Room Control Panel");
/* * * * * *  ESP-DASH Cards * * * * * * */

#define CARDLEN 8

/*Testing*/
Card touchval(&dashboard, GENERIC_CARD, "Hello World");
Card restart_master(&dashboard, BUTTON_CARD, "Restart Control"); //momentary


/* Tabs */
Tab attic(&dashboard, "Tabs!");




void configDash(){
  /* Configure ESP-Dash */
  /* Setup Tabs */
  touchval.setTab(&attic);
  // dashboard.setAuthentication("admin", "1234"); // Authentication
  dashboard.setTitle("Trees Example");

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


/* Restart This Master Server*/

restart_master.attachCallback([](int value){
buttonTimeout(&restart_master);
Serial.printf("Master Restart Triggered\n");
ESP.restart();

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



void setup() {
  Serial.begin(115200);
//LED 2 is the blue onboard light of the ESP32
  pinMode(2, OUTPUT);
  digitalWrite(2,LOW);

  digitalWrite(2,HIGH);
  //This makes it turn on.


  /* Start Wifi and ESP-DASH*/
  startWifi();
  configDash();
  startButtonCB();
}

unsigned long ttime = millis();

void loop() {

//CLEAN CLEAN CLEAN THIS LINE
if (millis() - ttime > 2000){
  Serial.println("Tick Tock");
  WebSerial.println(analogRead(34));
  ttime = millis();
}

  asynctimer.handle();
}