/*
  -----------------------
  Escape Room Master Server
  Adriaan van Wijk
  22 May 2023
  This Code is licenced under the 

  This code is for a server which listens for messages from the clients and sends control override messages 
  to clients in an escape room setup




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

*/



/* Kernal*/
#include <Arduino.h>

/* ESP-DASH */
#include <ESPDashPro.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

/* Wifi and mDNS */
#include <WiFi.h>
#include <ESPmDNS.h>

/* ESP-NOW */
#include <esp_now.h>


/* WiFi Credentials */
const char* ssid = "Mourning to Dancing"; // SSID
const char* password = "throughchristalone"; // Password




/* Setup */
AsyncWebServer server(80);
ESPDash dashboard(&server,false);

// ESPDash dashboard.setTitle("Escape Room Control Panel");
/* * * * * *  ESP-DASH Cards * * * * * * */


/* Attic */
Tab attic(&dashboard, "Attic");
Card humanchaindoor(&dashboard, BUTTON_CARD, "Open Human Chain Door");
Card bikelight(&dashboard, BUTTON_CARD, "Bicycle Lightbulb Override");
Card clockmotor(&dashboard, BUTTON_CARD, "Clock Motor Override");

/* Ancient Tomb */
Tab tomb(&dashboard, "Ancient Tomb");
Card beetle(&dashboard, BUTTON_CARD, "Open Beetle Puzle");
Card chalicedoor(&dashboard, BUTTON_CARD, "Open Chalice Door");
Card ringreader(&dashboard, BUTTON_CARD, "Override Ring Reader");
Card tangrumtomb(&dashboard, BUTTON_CARD, "Tangrum Puzzle Override");



/* All Aboard */
Tab train(&dashboard, "All Aboard");
Card thumbreaderdoor(&dashboard, BUTTON_CARD, "Open Thumb Reader Door");



void setup() {
  Serial.begin(115200);

 /* Setup Tabs */
  // dashboard.setAuthentication("admin", "1234"); // Authentication
  dashboard.setTitle("Escape Room Master Control");

  /* Attic */
  humanchaindoor.setTab(&attic);
  bikelight.setTab(&attic);
  clockmotor.setTab(&attic);

  /* Tomb */
  beetle.setTab(&tomb);
  chalicedoor.setTab(&tomb);
  ringreader.setTab(&tomb);
  tangrumtomb.setTab(&tomb);

  /* Train */
  thumbreaderdoor.setTab(&train);


/* Connect WiFi */
  WiFi.mode(WIFI_STA);
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
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  if (!MDNS.begin("escape")) {
        Serial.println("Error setting up MDNS responder!");
        while(1) {
            delay(1000);
        }
    }
  Serial.println("mDNS responder started");

  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
}