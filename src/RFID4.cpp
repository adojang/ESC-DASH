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


// #define SS_PIN  5  // ESP32 pin GPIO5 
#define RST_PIN 5 // ESP32 pin GPIO21



// #define SS_1_PIN        27x         // Configurable, take a unused pin, only HIGH/LOW required, must be different to SS 2
// #define SS_2_PIN        26x          // Configurable, take a unused pin, only HIGH/LOW required, must be different to SS 1
// #define SS_3_PIN        25x // Exclusion
// #define SS_4_PIN        33x
// #define SS_5_PIN        32 // 14 originally
// #define SS_6_PIN        13

#define SS_1_PIN        13         // Configurable, take a unused pin, only HIGH/LOW required, must be different to SS 2
#define SS_2_PIN        25 //EXCLUDED         // Configurable, take a unused pin, only HIGH/LOW required, must be different to SS 1
#define SS_3_PIN        26 
#define SS_4_PIN        27
#define SS_5_PIN        32 // 14 originally
#define SS_6_PIN        33

bool hex1 = false;
bool hex2 = true;
bool hex3 = false;
bool hex4 = false;
bool hex5 = false; // excluded for now.
bool hex6 = false;

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
 int HexCount = 0;

/* ESP-NOW Callback Functions*/

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&rData, incomingData, sizeof(rData));
  Serial.println("Override Data Recieved...");

if(rData.origin == atticmaster && rData.sensor == atticmaster && rData.data == 66){
  //Restart ESP and reset the counter.
  WebSerial.println("RESARTING ESP32...");
  ESP.restart();
}
  
  // Add your code here to do something with the data recieved.
  //It's probably best to use a flag instead of calling it directly here. Not Sure


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
  HexCount = 0;

  if (hex1 == true){
    HexCount = HexCount + 1;
  }

  if (hex2 == true){
     HexCount = HexCount + 1;
  }
  
  if (hex3 == true){
     HexCount = HexCount + 1;
  }

  if (hex4 == true){
     HexCount = HexCount + 1;
  }

  if (hex5 == true){
     HexCount = HexCount + 1;
  }

  if (hex6 == true){
     HexCount = HexCount + 1;
  }


    sData.data = HexCount;
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sData, sizeof(sData));
    if (result == ESP_OK) { Serial.println("Sent with success");}
    else {Serial.println("Error sending the data");}

    //Correct for 5/6 Later, if we ever get there.
}

void setup() {
  Serial.begin(115200);
  startWifi();
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

  sendData();
  Serial.println("Tap an RFID/NFC tag on the RFID-RC522 reader");
  WebSerial.println("Tap an RFID/NFC tag on the RFID-RC522 reader");



  //This line is sort of required. It automatically sends the data every 5 seconds. Don't know why. But hey there it is.
  // asynctimer.setInterval([]() {sendData();},  5000);
}



void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

unsigned long ttimer = millis();

void loop() {


  if(millis() - ttimer > 2000){
    WebSerial.println(HexCount);
    ttimer = millis();
  }

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

            if(uidText == "90bd4a26" && reader==0){
        Serial.println("Pin 13 Reader 0 Triggered.");
        WebSerial.println("Pin 13 Reader 0 Triggered.");
        hex1 = true;
        sendData();
      } 

      //TEMP Excluded by Wendy's Request (Bottom one)

      // if(uidText == "c32f19a0" && reader==1){
      //   Serial.println("Pin 25 Reader 2 Triggered.");
      //   hex2 = true;
      //   sendData();
      // } 

                  if(uidText == "901fd026" && reader==1){
        Serial.println("Pin 26 Reader 1 Triggered.");
        WebSerial.println("Pin 26 Reader 1 Triggered.");
        hex3 = true;
        sendData();
      }

                  if(uidText == "932f92d" && reader==2){
        Serial.println("Pin 27 Reader 2 Triggered.");
        WebSerial.println("Pin 27 Reader 2 Triggered.");
        hex4 = true;
        sendData();
      } 

                  if(uidText == "31cc8b" && reader==3){
        Serial.println("Pin 32 Reader 5 Triggered.");
        WebSerial.println("Pin 32 Reader 5 Triggered.");
        hex5 = true;
        sendData();
      } 

                  if(uidText == "919a1f1d" && reader == 4){
        Serial.println("Pin 33 Reader 5 Triggered.");
        WebSerial.println("Pin 33 Reader 5 Triggered.");
        
        hex6 = true;
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
  delay(25);








  //Required for the asynctimer to work.
  asynctimer.handle();
}