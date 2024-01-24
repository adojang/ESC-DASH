/*
--------------------------------------------------------------------------
                          Tygervallei Escape Room Project
--------------------------------------------------------------------------                          
  Author: Adriaan van Wijk
  Date: 24 January 2024

  This code is part of a multi-node project involving Escape Rooms in Tygervallei,
  South Africa.

  Copyright (c) 2023 Proxonics (Pty) Ltd

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at:

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
  --------------------------------------------------------------------------
*/

#include <EscCore.h>
#include <esp_task_wdt.h> // watchdog for doorlock mag recovery.
#define WDT_TIMEOUT 15 // 15 seconds

/* RFID */
#include <SPI.h>
#include <MFRC522.h>

#define NAME "RFID4"
#define setMACAddress m_RFID4

#pragma region mac
// Control
uint8_t m_masterserver[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x00};
uint8_t m_trainmaster[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x01};
uint8_t m_tombmaster[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x02};
uint8_t m_atticmaster[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x03};

// Attic
uint8_t m_attic_humanchain[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA0};
uint8_t m_attic_bike[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA1};
uint8_t m_clock[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA2};
uint8_t m_attic_overrideButton[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA3};
uint8_t m_RFID1[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA4};
uint8_t m_RFID2[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA5};
uint8_t m_RFID3[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA6};
uint8_t m_RFID4[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA7};
uint8_t m_attic_clock2[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA8};
uint8_t m_attic_morse[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xA9};


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
#pragma endregion mac

AsyncTimer asynctimer(35);
AsyncWebServer server(80);
ESPDash dashboard(&server,false);
esp_now_peer_info_t peerInfo;
EscCore Core;

dataPacket sData; // data to send
dataPacket rData; // data to recieve

/* Configuration and Setup */

#define RST_PIN 5 // ESP32 pin GPIO21
#define SS_1_PIN        13         // Configurable, take a unused pin, only HIGH/LOW required, must be different to SS 2
#define SS_2_PIN        25 //EXCLUDED         // Configurable, take a unused pin, only HIGH/LOW required, must be different to SS 1
#define SS_3_PIN        26 
#define SS_4_PIN        27
#define SS_5_PIN        32 // 14 originally
#define SS_6_PIN        33

#define NR_OF_READERS   6
byte ssPins[] = {SS_1_PIN, SS_2_PIN, SS_3_PIN, SS_4_PIN, SS_5_PIN, SS_6_PIN};
MFRC522 mfrc522[NR_OF_READERS];   // Create MFRC522 instance.

bool hex1 = false;
bool hex2 = false;
bool hex3 = false;
bool hex4 = false;
bool hex5 = false; 
bool hex6 = false;

int HexCount = 0;


/* Functions */

void sendData()
{
  HexCount = 0;

  if (hex1 == true){
    HexCount = HexCount + 1;
    WebSerial.printf("Hex1 | ");
  }

  if (hex2 == true){
     HexCount = HexCount + 1;
     WebSerial.printf("Hex2 | ");
  }
  
  if (hex3 == true){
     HexCount = HexCount + 1;
     WebSerial.printf("Hex3 | ");
  }

  if (hex4 == true){
     HexCount = HexCount + 1;
    WebSerial.printf("Hex4 | ");
  }

  if (hex5 == true){
     HexCount = HexCount + 1;
    WebSerial.printf("Hex5 | \n");
  }

  // if (hex6 == true){
  //    HexCount = HexCount + 1;
  //   WebSerial.println("Hex6 | \n");
  // }

    WebSerial.println(HexCount);
    sData.origin = attic_RFID4;
    sData.sensor = attic_RFID4;
    sData.data = HexCount;
    esp_err_t result = esp_now_send(m_atticmaster, (uint8_t *) &sData, sizeof(sData));
    if (result == ESP_OK) { Serial.println("Sent with success");}
    else {Serial.println("Error sending the data");}

  Serial.printf("Tiles: %d\n ", HexCount);

}


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // WebSerial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");
  }

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) 
{
  memcpy(&rData, incomingData, sizeof(rData));
  if(rData.origin == masterserver && rData.sensor == masterserver && rData.data == 77){
    Serial.println("Restarting");
    ESP.restart();
  }

    if(rData.origin == atticmaster && rData.sensor == atticmaster && rData.data == 0){
    sendData(); // refresh master Data after the atticmaster restarts.
    WebSerial.println("Send Data Refresh it");
  }
}


void startespnow(){ // Remeber to register mac addresses before sending data;
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
}

void registermac(uint8_t address[]){
  memcpy(peerInfo.peer_addr, address, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
}

void statusUpdate(){
  sData.origin = attic_RFID4;
  sData.sensor = status_alive;
  esp_err_t result = esp_now_send(m_masterserver, (uint8_t *) &sData, sizeof(sData));
  esp_task_wdt_reset();
}


void setup() {
  Serial.begin(115200);
  Serial.println("Configuring WDT...");
  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch
  
  Core.startup(setMACAddress, NAME, server);
  startespnow();
  registermac(m_masterserver);
  registermac(m_atticmaster);

  sData.origin = attic_RFID4;
  sData.sensor = attic_RFID4;

  pinMode(SS_1_PIN, PULLUP);
  pinMode(SS_2_PIN, PULLUP);
  pinMode(SS_3_PIN, PULLUP);
  pinMode(SS_4_PIN, PULLUP);
  pinMode(SS_5_PIN, PULLUP);
  pinMode(SS_6_PIN, PULLUP);

  SPI.begin();        // Init SPI bus

  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN); // Init each MFRC522 card
    delay(10);
    mfrc522[reader].PCD_SetRegisterBitMask(mfrc522[reader].RFCfgReg, (0x07<<4));
    delay(10);
    Serial.print(F("Reader "));
    Serial.print(reader);
    Serial.print(F(": "));
        if(mfrc522[reader].PCD_DumpVersionToSerial() == 1){
      Serial.println("ERROR, RESTARTING ESP TO HOPEFULLY CLEAR UP.");
      ESP.restart();
    };
  }

  sendData(); // On boot send data to reset counter to 0 on masterserver.
  Serial.println("Tap an RFID/NFC tag on the RFID-RC522 reader");
  WebSerial.println("Tap an RFID/NFC tag on the RFID-RC522 reader");

  asynctimer.setInterval([]() {statusUpdate();},  1000);
  asynctimer.setInterval([]() {sendData();},  1000);
}


void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

bool rfid_tag_present_prev[6] = {false,false,false,false,false,false};
bool rfid_tag_present[6] = {false,false,false,false,false,false};
int _rfid_error_counter[6] ={0,0,0,0,0,0};
bool _tag_found[6] = {false,false,false,false,false,false};

void handleRFID(){
  
for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
  String uidText = "";

    rfid_tag_present_prev[reader] = rfid_tag_present[reader];
    _rfid_error_counter[reader] += 1;
  if(_rfid_error_counter[reader] > 2){
    _tag_found[reader] = false;
  }

  // Detect Tag without looking for collisions
  byte bufferATQA[2];
  byte bufferSize = sizeof(bufferATQA);

  // Reset baud rates
  mfrc522[reader].PCD_WriteRegister(mfrc522[reader].TxModeReg, 0x00);
  mfrc522[reader].PCD_WriteRegister(mfrc522[reader].RxModeReg, 0x00);
  // Reset ModWidthReg
  mfrc522[reader].PCD_WriteRegister(mfrc522[reader].ModWidthReg, 0x26);

  MFRC522::StatusCode result = mfrc522[reader].PICC_RequestA(bufferATQA, &bufferSize);

  if(result == mfrc522[reader].STATUS_OK){
    if ( ! mfrc522[reader].PICC_ReadCardSerial()) {
       //Since a PICC placed get Serial and continue   
      return;
    }
    _rfid_error_counter[reader] = 0;
    _tag_found[reader] = true;        
  
  // dump_byte_array(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size);

  for (byte i = 0; i < mfrc522[reader].uid.size; i++) {
        uidText += String(mfrc522[reader].uid.uidByte[i], HEX);
      }
      // Serial.println();
      // Serial.printf("Reader: %d\n", reader);
      // Serial.println(uidText);
  
  }
  
  rfid_tag_present[reader] = _tag_found[reader];
  
  // rising edge
  if (rfid_tag_present[reader] && !rfid_tag_present_prev[reader])
  {

    if (uidText == "90bd4a26" && reader == 0)
    {
      // Serial.println("Pin 13 Reader 0 Triggered.");
      // WebSerial.println("Pin 13 Reader 0 Triggered.");
      hex1 = true;
    }

    if (uidText == "901fd026" && reader == 1)
    {
      // Serial.println("Pin 25 Reader 2 Triggered.");
      hex2 = true;
    }

    // THIS RFID READER IS NOT USED.
    if (uidText == "932f92d" && reader == 2)
    {
      // Serial.println("Pin 26 Reader 1 Triggered.");
      // WebSerial.println("Pin 26 Reader 1 Triggered.");
      hex3 = true;
    }
    if (uidText == "31cc8b" && reader == 3)
    {
      // Serial.println("Pin 27 Reader 2 Triggered.");
      // WebSerial.println("Pin 27 Reader 2 Triggered.");
      hex4 = true;
    }
    if (uidText == "919a1f1d" && reader == 4)
    {
      // Serial.println("Pin 32 Reader 5 Triggered.");
      // WebSerial.println("Pin 32 Reader 5 Triggered.");
      hex5 = true;
    }
    // UNUSEDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD

    if (uidText == "c32f19a0" && reader == 5)
    {
      // Serial.println("Pin 33 Reader 5 Triggered.");
      // WebSerial.println("Pin 33 Reader 5 Triggered.");

      hex6 = true;
    }
  }

  // falling edge
  if (!rfid_tag_present[reader] && rfid_tag_present_prev[reader]){
    // Serial.println("Tag gone");
   
         if( reader==0){
        // Serial.println("Pin 13 Reader 1 Triggered.");
        hex1 = false;
      } 

      if(reader==1){
        // Serial.println("Pin 14 Reader 2 Triggered.");
        hex2 = false;
      }

      if (reader == 2)
      {
        // Serial.println("Pin 27 Reader 3 Triggered.");
        hex3 = false;
      }

      if (reader == 3)
      {
        hex4 = false;
      }
      if (reader == 4)
      {
        hex5 = false;
      }
      if (reader == 5)
      {
        hex6 = false;
      }
  }

} // For loop
}


void loop() {

  handleRFID();

  asynctimer.handle();
}