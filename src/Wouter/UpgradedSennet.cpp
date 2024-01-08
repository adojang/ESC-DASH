/*
--------------------------------------------------------------------------
                          Tygervallei Escape Room Project
--------------------------------------------------------------------------
  Author: Adriaan van Wijk
  Date: 8 January 2024

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

#define NAME "sennet"
#define setMACAddress m_tomb_sennet
#define WDT_TIMEOUT 10 // 10 seconds
/* RFID */
#include <SPI.h>
#include <MFRC522.h>

/* Configuration and Setup */

#define RST_PIN 5
#define SS_1_PIN 13
#define SS_2_PIN 25
#define SS_3_PIN 26
#define SS_4_PIN 27

#define NR_OF_READERS 4

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
uint8_t m_tomb_sennet[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0xB0}; // D4:D4:DA:E3:A4:20 check
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
ESPDash dashboard(&server, false);
esp_now_peer_info_t peerInfo;
EscCore Core;
dataPacket sData;
dataPacket rData;

byte ssPins[] = {SS_1_PIN, SS_2_PIN, SS_3_PIN, SS_4_PIN};
MFRC522 mfrc522[NR_OF_READERS]; // Create MFRC522 instance.

/* Functions */
void trigger()
{
  digitalWrite(4, LOW);
  delay(500);
  digitalWrite(4, HIGH);
}

void printbyteArray(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  memcpy(&rData, incomingData, sizeof(rData));

  if (rData.origin == masterserver && rData.sensor == tomb_sennet && rData.data == 1)
  {
    trigger();
  }
}

void startespnow()
{ // Remeber to register mac addresses before sending data;
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
}

void registermac(uint8_t address[])
{
  memcpy(peerInfo.peer_addr, address, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
}

void statusUpdate()
{
  sData.origin = tomb_sennet;
  sData.sensor = status_alive;
  esp_err_t result = esp_now_send(m_masterserver, (uint8_t *)&sData, sizeof(sData));
  esp_task_wdt_reset(); // restarts out after 10 seconds of not sending.
}

void setup()
{
  Serial.begin(115200);

  // You cannot pullup all of these.

  pinMode(SS_1_PIN, PULLUP);
  pinMode(SS_2_PIN, PULLUP);
  pinMode(SS_3_PIN, PULLUP);
  pinMode(SS_4_PIN, PULLUP);

#pragma endregion gpio

  Core.startup(setMACAddress, NAME, server);
  startespnow();
  registermac(m_masterserver);
  registermac(m_tombmaster);
  asynctimer.setInterval([]()
                         { statusUpdate(); },
                         1000);
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);

  
  Serial.println("Configuring WDT...");
  esp_task_wdt_init(WDT_TIMEOUT, true); // enable panic so ESP32 restarts
  esp_task_wdt_add(NULL);               // add current thread to WDT watch

  SPI.begin(); // Init SPI bus


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


  sData.origin = tomb_tangrum;
  sData.sensor = tomb_tangrum;
  Serial.println("Tap an RFID/NFC tag on the RFID-RC522 reader");
}

unsigned long ttimer = millis();
int total = 0;
bool oneshotEnable = true;
int shotcnt = 0;

unsigned long oneshottimer = millis();

unsigned long timer1sec = millis();

void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

bool rfid_tag_present_prev[4] = {false,false,false,false};
bool rfid_tag_present[4] = {false,false,false,false};
int _rfid_error_counter[4] ={0,0,0,0};
bool _tag_found[4] = {false,false,false,false};

void loop()
{

  if (millis() - timer1sec > 1000)
  {

    WebSerial.printf("Seconds Online: %d\n", millis() / 1000);
    WebSerial.printf("Pawns Present: %d\n", total);
    timer1sec = millis();
  }


//RFID Core Code.


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
      // Serial.println(uidText);
  
  }
  
  rfid_tag_present[reader] = _tag_found[reader];
  
  // rising edge
  if (rfid_tag_present[reader] && !rfid_tag_present_prev[reader]){
    // Serial.println("Tag found");
   total +=1;
  }
  
  // falling edge
  if (!rfid_tag_present[reader] && rfid_tag_present_prev[reader]){
    total += 0;
  }

} // For loop

//At the end of each loop, reset total to 0;
    if ((total == 4) && shotcnt == 0)
    {
      Serial.printf("Puzzle Complete! 4/4\n");
      trigger();
      shotcnt = 1;
    }

        if ((total < 4) && shotcnt == 1)
    {
      shotcnt = 0;
    }

    // Restart ESP every two minutes and hope for dear life it works.
    // It will only restart if all pieces are not present or are removed.
    if (total == 0 && (millis() - oneshottimer > 120000))
    {
      Serial.printf("RELAY ARMED\n", total);
      // oneshottimer = millis(); //irrelavent
      ESP.restart();
      // shotcnt = 0; //irrelavent, since we are restarting, and this code will never actually
    }



  // total = 0;
  // for (uint8_t reader = 0; reader < NR_OF_READERS; reader++)
  // {
  //   // Look for new cards

  //   if (mfrc522[reader].PICC_IsCardPresent())
  //     total += 1;

  //   if ((total == 4) && shotcnt == 0)
  //   {
  //     // ttimer = millis();
  //     // oneshotEnable = false;

  //     Serial.printf("Puzzle Complete! 4/4\n");
  //     trigger();
  //     shotcnt = 1;
  //   }

  //   // Restart ESP every two minutes and hope for dear life it works.
  //   // It will only restart if all pieces are not present or are removed.
  //   if (total == 0 && (millis() - oneshottimer > 120000))
  //   {
  //     Serial.printf("RELAY ARMED\n", total);
  //     // oneshottimer = millis(); //irrelavent
  //     ESP.restart();
  //     // shotcnt = 0; //irrelavent, since we are restarting, and this code will never actually
  //   }

  //   MFRC522::PICC_Type piccType = mfrc522[reader].PICC_GetType(mfrc522[reader].uid.sak);
  //   mfrc522[reader].PICC_HaltA();
  //   mfrc522[reader].PCD_StopCrypto1();
  // }

  // delay(5); // This delay might cause in problems when multiple readers are used...
  
  
  
  
  total = 0; // Reset count.
  
  asynctimer.handle();
}