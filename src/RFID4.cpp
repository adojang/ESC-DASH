/*
/*
  --------------------------------------------------------------------------
  Escape Room Template
  Adriaan van Wijk
  22 May 2023

  MULTI-RFID Template To Use
  It is important to note that PULLUP has to be selected. Otherwise the CS (SS) floats :(

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

#define NAME "RFID4"
#define MACAD 0xA7 // Refer to Table in Conventions

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

/* RFID */
#include <SPI.h>
#include <MFRC522.h>


// #define SS_PIN  5  // ESP32 pin GPIO5 
#define RST_PIN 5 // ESP32 pin GPIO21

#define SS_1_PIN        27         // Configurable, take a unused pin, only HIGH/LOW required, must be different to SS 2
#define SS_2_PIN        26          // Configurable, take a unused pin, only HIGH/LOW required, must be different to SS 1
#define SS_3_PIN        25
#define SS_4_PIN        33
#define SS_5_PIN        32
#define SS_6_PIN        13

bool hex1 = false;
bool hex2 = false;
bool hex3 = false;

#define NR_OF_READERS   6

byte ssPins[] = {SS_1_PIN, SS_2_PIN, SS_3_PIN, SS_4_PIN, SS_5_PIN, SS_6_PIN};

MFRC522 mfrc522[NR_OF_READERS];   // Create MFRC522 instance.

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x03}; // Address of Master Server
uint8_t setMACAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, MACAD};


/* ESP Async Timer */
AsyncTimer asynctimer;

/* WiFi Credentials */
const char* ssid = WIFI_SSID; // SSID
const char* password = WIFI_PASS; // Password

/* ESP-NOW Structures */

dataPacket sData; // data to send
dataPacket rData; // data to recieve

/* Setup */
AsyncWebServer server(80);
esp_now_peer_info_t peerInfo;


bool turnlighton = false;


/* ESP-NOW Callback Functions*/

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&rData, incomingData, sizeof(rData));
  Serial.println("Override Data Recieved...");

  if (rData.data == 1) {
  //You should never use delay in this function. It might cause the ESP-NOW to crash.
    turnlighton = true;
  }
  else
  {
    turnlighton = false;
  }
  
  // Add your code here to do something with the data recieved.
  //It's probably best to use a flag instead of calling it directly here. Not Sure


}
 

void startwifi(){

  // Set device as a Wi-Fi Station
  WiFi.softAP(NAME, "pinecones", 0, 1, 4);
  WiFi.mode(WIFI_AP_STA);
  esp_wifi_set_mac(WIFI_IF_STA, &setMACAddress[0]);
  delay(250);
  WiFi.begin(ssid, password);
  delay(250);
  unsigned long wifitimeout = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
    if (millis()- wifitimeout > 5000){
      Serial.printf("WiFi Failed to Connect!\n"); // 10 second timeout
      ESP.restart();
    } 
  }
    Serial.println("WIFI CONNECTED!");

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

void sendData()
{
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sData, sizeof(sData));
      if (result == ESP_OK) { Serial.println("Sent with success");}
      else {Serial.println("Error sending the data");}

}

void setup() {
  Serial.begin(115200);
  startwifi();
  startespnow();
  sData.origin = attic_RFID4;
  sData.sensor = attic_RFID4;

  pinMode(SS_1_PIN, PULLUP);
  pinMode(SS_2_PIN, PULLUP);
  pinMode(SS_3_PIN, PULLUP);
  pinMode(SS_4_PIN, PULLUP);
  pinMode(SS_5_PIN, PULLUP);
  pinMode(SS_6_PIN, PULLUP);

  

  //Make any Edits you need to add below this line ------------------------------

  pinMode(2, OUTPUT);
  digitalWrite(2,LOW);


  SPI.begin();        // Init SPI bus

  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN); // Init each MFRC522 card
    delay(10);
    mfrc522[reader].PCD_SetRegisterBitMask(mfrc522[reader].RFCfgReg, (0x07<<4));
    delay(10);
    Serial.print(F("Reader "));
    Serial.print(reader);
    Serial.print(F(": "));
    mfrc522[reader].PCD_DumpVersionToSerial();
  }


  Serial.println("Tap an RFID/NFC tag on the RFID-RC522 reader");


  //This line is sort of required. It automatically sends the data every 5 seconds. Don't know why. But hey there it is.
  // asynctimer.setInterval([]() {sendData();},  5000);
}



void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void loop() {

  if (turnlighton) {
    digitalWrite(2,HIGH);
  }
  else {
    digitalWrite(2,LOW);
  }

  
 for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    // Look for new cards

    if (mfrc522[reader].PICC_IsNewCardPresent() && mfrc522[reader].PICC_ReadCardSerial()) {
      Serial.print(F("Reader "));
      Serial.print(reader);
      // Show some details of the PICC (that is: the tag/card)
      Serial.print(F(": Card UID:"));
      dump_byte_array(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size);
      Serial.println();
      Serial.print(F("PICC type: "));
      MFRC522::PICC_Type piccType = mfrc522[reader].PICC_GetType(mfrc522[reader].uid.sak);
      Serial.println(mfrc522[reader].PICC_GetTypeName(piccType));

      sData.data = 1;
      sendData();
      sData.data = 0;
      // Halt PICC
      mfrc522[reader].PICC_HaltA();
      // Stop encryption on PCD
      mfrc522[reader].PCD_StopCrypto1();
    } //if (mfrc522[reader].PICC_IsNewC
  } //for(uint8_t reader
  asynctimer.handle();

}







  //Required for the asynctimer to work.
