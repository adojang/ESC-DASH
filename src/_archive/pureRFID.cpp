/*
--------------------------------------------------------------------------
                          Tygervallei Escape Room Project
--------------------------------------------------------------------------                          
  Author: Adriaan van Wijk
  Date: 16 October 2023

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

/* RFID */
#include <SPI.h>
#include <MFRC522.h>



/* Configuration and Setup */

#define RST_PIN         5 
#define SS_1_PIN        13
#define SS_2_PIN        25 
#define SS_3_PIN        26 
#define SS_4_PIN        27

#define NR_OF_READERS   4
byte ssPins[] = {SS_1_PIN, SS_2_PIN, SS_3_PIN, SS_4_PIN};
MFRC522 mfrc522[NR_OF_READERS];   // Create MFRC522 instance.

bool hex1 = false;
bool hex2 = false;
bool hex3 = false;
bool hex4 = false;

int HexCount = 0;


/* Functions */

void printbyteArray(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}



void setup() {
  Serial.begin(115200);

    //You cannot pullup all of these.

  pinMode(SS_1_PIN, PULLUP);
  pinMode(SS_2_PIN, PULLUP);
  pinMode(SS_3_PIN, PULLUP);
  pinMode(SS_4_PIN, PULLUP);

  
  pinMode(4,OUTPUT);
  digitalWrite(4,HIGH);

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
}


void trigger(){
    digitalWrite(4,LOW);
    delay(500);
    digitalWrite(4,HIGH);
}



unsigned long ttimer = millis();
int total = 0;
bool oneshotEnable = true;
int shotcnt = 0;

unsigned long oneshottimer = millis();


void loop() {

  
    total = 0;
 for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    // Look for new cards

    if (mfrc522[reader].PICC_IsCardPresent()) total += 1;

    
    if ((total == 4) && shotcnt == 0){
        //ttimer = millis();
        //oneshotEnable = false;

        Serial.printf("Puzzle Complete! 4/4\n");
        trigger();
        shotcnt = 1;

}

//This arms the relay again so it will work.
  if(total == 0 && (millis() - oneshottimer > 30000)){
    Serial.printf("RELAY ARMED\n", total);
    oneshottimer = millis();
    shotcnt = 0;

}




    
    // if (mfrc522[reader].PICC_IsNewCardPresent() && mfrc522[reader].PICC_ReadCardSerial()) {
    //   } 

        MFRC522::PICC_Type piccType = mfrc522[reader].PICC_GetType(mfrc522[reader].uid.sak);
        mfrc522[reader].PICC_HaltA();
        mfrc522[reader].PCD_StopCrypto1();
     
  }

  delay(5); // This delay might cause in problems when multiple readers are used...

}