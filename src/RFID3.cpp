/*
/*
  --------------------------------------------------------------------------
  Escape Room Template
  Adriaan van Wijk
  22 May 2023

  MULTI-RFID Template To Use

  // Pin 27 #3
  // Pin 14 # 2
  // Pin 13 #1

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

#define NAME "RFID3"
#define MACAD 0xA6 // Refer to Table in Conventions

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

#define SS_1_PIN        13         // Configurable, take a unused pin, only HIGH/LOW required, must be different to SS 2
#define SS_2_PIN        14          // Configurable, take a unused pin, only HIGH/LOW required, must be different to SS 1
#define SS_3_PIN        27 //14

bool hex1 = false;
bool hex2 = false;
bool hex3 = false;

#define NR_OF_READERS   3

byte ssPins[] = {SS_1_PIN, SS_2_PIN, SS_3_PIN};

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
  int HexCount = 0;

  if (hex1 == true){
    HexCount = HexCount + 1;
  }

  if (hex2 == true){
     HexCount = HexCount + 1;
  }
  
  if (hex3 == true){
     HexCount = HexCount + 1;
  }


    sData.data = HexCount;
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sData, sizeof(sData));
    if (result == ESP_OK) { Serial.println("Sent with success");}
    else {Serial.println("Error sending the data");}

    if (sData.data == 3){
      //RESET
      delay(3000);
      hex1 = false;
      hex2 = false;
      hex3 = false;
      sData.data = 0;
    // esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sData, sizeof(sData));
    // if (result == ESP_OK) { Serial.println("Sent with success");}
    // else {Serial.println("Error sending the data");}
    }

}

void setup() {
  Serial.begin(115200);
  startwifi();
  startespnow();
  sData.origin = attic_RFID3;
  sData.sensor = attic_RFID3;

  //Make any Edits you need to add below this line ------------------------------

  pinMode(SS_1_PIN, PULLUP);
  pinMode(SS_2_PIN, PULLUP);
  pinMode(SS_3_PIN, PULLUP);
  pinMode(2, OUTPUT);
  digitalWrite(2,LOW);


  SPI.begin();        // Init SPI bus

  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN); // Init each MFRC522 card
    mfrc522[reader].PCD_SetRegisterBitMask(mfrc522[reader].RFCfgReg, (0x07<<4));
    Serial.print(F("Reader "));
    Serial.print(reader);
    Serial.print(F(": "));
    mfrc522[reader].PCD_DumpVersionToSerial();
  }

  //Initialize Data as 0;
  sData.data = 0;
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sData, sizeof(sData));

  Serial.println("Tap an RFID/NFC tag on the RFID-RC522 reader");

  //This resets the counter on the main webpage.
  esp_err_t result2 = esp_now_send(broadcastAddress, (uint8_t *) &sData, sizeof(sData));

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
      // Serial.print(F("Reader "));
      // Serial.print(reader);
      // Show some details of the PICC (that is: the tag/card)
      // Serial.print(F(": Card UID:"));
      dump_byte_array(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size);
      
      String uidText = "";
      for (byte i = 0; i < mfrc522[reader].uid.size; i++) {
        uidText += String(mfrc522[reader].uid.uidByte[i], HEX);
      }
      Serial.println();
      Serial.println(uidText);

            if(uidText == "9093a226" && reader==0){
        Serial.println("Pin 13 Reader 1 Triggered.");
        hex1 = true;
        sendData();
      } 

      if(uidText == "90d0126" && reader==1){
        Serial.println("Pin 14 Reader 2 Triggered.");
        hex2 = true;
        sendData();
      } 

            if(uidText == "90b99e26" && reader==2){
        Serial.println("Pin 27 Reader 3 Triggered.");
        hex3 = true;
        sendData();
      } 
      
      //  Serial.print(F("PICC type: "));
       MFRC522::PICC_Type piccType = mfrc522[reader].PICC_GetType(mfrc522[reader].uid.sak);
      //  Serial.println(mfrc522[reader].PICC_GetTypeName(piccType));

      // Halt PICC
      mfrc522[reader].PICC_HaltA();
      // Stop encryption on PCD
      mfrc522[reader].PCD_StopCrypto1();
    } //if (mfrc522[reader].PICC_IsNewC
  } //for(uint8_t reader
  delay(50);








  //Required for the asynctimer to work.
  asynctimer.handle();
}