/*
  --------------------------------------------------------------------------
  Escape Room Template
  Adriaan van Wijk
  22 May 2023

  This code is part of a multi-node project which involes Esc Rooms in Tygervallei,
  South Africa.

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

#include <EscCore.h>
#include <Adafruit_Keypad.h>

#define NAME "keypad"
#define setMACAddress m_train_keypad

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

unsigned long ttime = millis();
// Keypad Variables

/* Setup Keypad*/
const byte ROWS = 4; // rows
const byte COLS = 3; // columns
// define the symbols on the buttons of the keypads
char keys[ROWS][COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};
// ESP RIGHT SIDE
byte rowPins[ROWS] = {19, 4, 16, 5}; // connect to the row pinouts of the keypad
byte colPins[COLS] = {18, 21, 17};   // connect to the column pinouts of the keypad

byte rowPins2[ROWS] = {14, 32, 33, 26}; // connect to the row pinouts of the keypad
byte colPins2[COLS] = {27, 12, 25};   // connect to the column pinouts of the keypad

// C2 R1 C1 R4 C3 R3 R2
//12 14 27 26 25 33 32  

// initialize an instance of class NewKeypad
Adafruit_Keypad customKeypad = Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
Adafruit_Keypad customKeypad2 = Adafruit_Keypad(makeKeymap(keys), rowPins2, colPins2, ROWS, COLS);

/*Keypad Password */
const String correctSequence = "1617";
String enteredSequence = "";
unsigned long lastKeyPressTimestamp = 0;
const unsigned long resetTimeout = 10000; // 10 seconds

const String correctSequence2 = "1379";
String enteredSequence2 = "";
unsigned long lastKeyPressTimestamp2 = 0;
const unsigned long resetTimeout2 = 10000; // 10 seconds


/* Functions */



void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {WebSerial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    memcpy(&rData, incomingData, sizeof(rData));

    if (rData.origin == masterserver && rData.sensor == masterserver)
    {
      if (rData.data == 1) // Door Open
      {
        digitalWrite(2,HIGH); // Turn off Red LED
        digitalWrite(22,HIGH);

        digitalWrite(15,LOW); // Turn on Green LED Steady.
        digitalWrite(23,LOW);

      }
      
      if (rData.data == 0) // Door Closed
      {
          digitalWrite(15,HIGH); // Disable Solid Green
          digitalWrite(23,HIGH);
          
          digitalWrite(2,LOW); // Enable Solid RED LED.
          digitalWrite(22,LOW);

      }

    }
  



}


/*Keypad Password */
bool checkSequence() {
  return enteredSequence == correctSequence;
}
bool checkSequence2() {
  return enteredSequence2 == correctSequence2;
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
  sData.origin = train_keypad;
  sData.sensor = status_alive;
  esp_err_t result = esp_now_send(m_masterserver, (uint8_t *) &sData, sizeof(sData));
}

void flashRED(){
  sData.origin = train_keypad;
  sData.sensor = train_keypad;
  sData.data = 2;
  esp_err_t result = esp_now_send(m_masterserver, (uint8_t *) &sData, sizeof(sData));
  digitalWrite(15,HIGH); // Disable Solid Green
  digitalWrite(23,HIGH);
  //Flash Red LED Many Times
  for (int i=0;i<8;i++){
  digitalWrite(2,LOW);
  digitalWrite(22,LOW);
  delay(100);
  digitalWrite(2,HIGH);
  digitalWrite(22,HIGH);
  delay(100);
}
digitalWrite(22,LOW);
digitalWrite(2,LOW); // Enable Solid RED LED.
}

void sendData()
{
  sData.origin = train_keypad;
  sData.sensor = train_keypad;
  sData.data = 1;

  esp_err_t result = esp_now_send(m_masterserver, (uint8_t *) &sData, sizeof(sData));


  digitalWrite(2,HIGH); // Turn of Red LED
  digitalWrite(22,HIGH);

  for (int i=0;i<8;i++){ // Flash Green LED
    digitalWrite(15,LOW);
    digitalWrite(23,LOW);
    delay(100);
    digitalWrite(15,HIGH);
    digitalWrite(23,HIGH);
    delay(100);
  }
  digitalWrite(15,LOW); // Turn on Green LED Steady.
  digitalWrite(23,LOW);
}


void setup() {
  Serial.begin(115200);

  Core.startup(setMACAddress, NAME, server);
  startespnow();
  registermac(m_masterserver);

  sData.origin = train_keypad;
  sData.sensor = train_keypad;

  #pragma region gpio
    pinMode(2, OUTPUT); // RED LED Inside
    pinMode(15, OUTPUT); // Green LED Inside
    pinMode(22,OUTPUT); // X LED Outside
    pinMode(23,OUTPUT); // X LED Outisde
    pinMode(13,OUTPUT); // LED PWR
    digitalWrite(13,HIGH); // LED Outside PWR
    digitalWrite(22,LOW); //RED Outside LED
    digitalWrite(2,LOW); // RED INSIDE LED

    digitalWrite(15,HIGH); // Green LED HIGH means DISABLED
    digitalWrite(23,HIGH); // Green Outside LED
  #pragma endregion gpio

  customKeypad.begin(); // Startup for Keypad
  customKeypad2.begin(); // Startup for Keypad


  asynctimer.setInterval([]() {statusUpdate();},  1000);

}


void loop() {
 customKeypad.tick();
  customKeypad2.tick();

  while (customKeypad.available())
  {
    keypadEvent e = customKeypad.read();
    Serial.print((char)e.bit.KEY);
    if (e.bit.EVENT == KEY_JUST_PRESSED) {
      char key = (char)e.bit.KEY;
      if (key == '#') {
        if (checkSequence()) {
          Serial.println("Correct sequence entered!");
          sendData();

        } else {
          Serial.println("WRONG");
          flashRED();
        }
        enteredSequence = ""; // Reset the entered sequence after checking
      } else {
        enteredSequence += key;
      }
      lastKeyPressTimestamp = millis();
      Serial.println(" pressed");
    } else if (e.bit.EVENT == KEY_JUST_RELEASED) {
      Serial.println(" released");
    }
  }

  while (customKeypad2.available())
  {
    keypadEvent e2 = customKeypad2.read();
    Serial.print((char)e2.bit.KEY);
    if (e2.bit.EVENT == KEY_JUST_PRESSED) {
      char key2 = (char)e2.bit.KEY;
      if (key2 == '#') {
        if (checkSequence2()) {
          Serial.println("Correct sequence entered!");
          sendData();
        } else {
          Serial.println("WRONG");
          flashRED();
        }
        enteredSequence2 = ""; // Reset the entered sequence after checking
      } else {
        enteredSequence2 += key2;
      }
      lastKeyPressTimestamp2 = millis();
      Serial.println(" pressed");
    } else if (e2.bit.EVENT == KEY_JUST_RELEASED) {
      Serial.println(" released");
    }
  }


  // Reset entered sequence if there's no activity for 20 seconds
  if (millis() - lastKeyPressTimestamp >= resetTimeout) {
    Serial.println("Resetting sequence due to timeout");
    enteredSequence = "";
    lastKeyPressTimestamp = millis();
  }

    if (millis() - lastKeyPressTimestamp2 >= resetTimeout2) {
    Serial.println("Resetting sequence 2 due to timeout");
    enteredSequence2 = "";
    lastKeyPressTimestamp2 = millis();
  }

  // Required for the asynctimer to work.
  asynctimer.handle();
}