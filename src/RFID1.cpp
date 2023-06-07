/*
/*
  --------------------------------------------------------------------------
  Escape Room Template
  Adriaan van Wijk
  22 May 2023

  SINGLE RFID Template To Use

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

#define NAME "RFID1"
#define MACAD 0xA4 // Refer to Table in Conventions

#define SS_PIN  5  // ESP32 pin GPIO5 
#define RST_PIN 21 // ESP32 pin GPIO21




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




MFRC522 rfid(SS_PIN, RST_PIN);


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

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.printf("WiFi Failed!\n");
      ESP.restart();
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

  sData.origin = attic_RFID1;
  sData.sensor = attic_RFID1;

  // 
  pinMode(SS_PIN, PULLUP);

  //Make any Edits you need to add below this line ------------------------------

  pinMode(2, OUTPUT);
  digitalWrite(2,LOW);


  SPI.begin(); // init SPI bus
  rfid.PCD_Init(); // init MFRC522
  // Enhance the MFRC522 Receiver Gain to maximum value of some 48 dB
  rfid.PCD_SetRegisterBitMask(rfid.RFCfgReg, (0x07<<4));

  Serial.println("Tap an RFID/NFC tag on the RFID-RC522 reader");


  //This line is sort of required. It automatically sends the data every 5 seconds. Don't know why. But hey there it is.
  // asynctimer.setInterval([]() {sendData();},  5000);
}




void loop() {

  if (turnlighton) {
    digitalWrite(2,HIGH);
  }
  else {
    digitalWrite(2,LOW);
  }


  if (rfid.PICC_IsNewCardPresent()) { // new tag is available
    if (rfid.PICC_ReadCardSerial()) { // NUID has been readed
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      Serial.print("RFID/NFC Tag Type: ");
      Serial.println(rfid.PICC_GetTypeName(piccType));
      sData.data = 1;
      sendData();
  
      sData.data = 0;
      // print UID in Serial Monitor in the hex format
      Serial.print("UID:");
      for (int i = 0; i < rfid.uid.size; i++) {
        Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(rfid.uid.uidByte[i], HEX);
      }
      Serial.println();

      rfid.PICC_HaltA(); // halt PICC
      rfid.PCD_StopCrypto1(); // stop encryption on PCD
    }
  }



  //Required for the asynctimer to work.
  asynctimer.handle();
}