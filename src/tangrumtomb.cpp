/*
  --------------------------------------------------------------------------
  Escape Room Template
  Adriaan van Wijk
  22 May 2023

  Give a short explaination of what this code does and for what puzzle it is.

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

#define NAME "tangrumtomb"
#define MACAD 0x07 // Refer to Table Below

/* Data Naming Convention for Mac Addresses
*  0x00 - masterserver
 * 0x01 - humanchain
 * 0x02 - bikelight
 * 0x03 - clockmotor
 * 0x04 - beetle
 * 0x05 - chalicedoor
 * 0x06 - ringreader
 * 0x07 - tangrumtomb
 * 0x08 - thumbreaderdoor
*/

/* Kernal*/
#include <Arduino.h>
#include <config.h>

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

/* SET MAC ADDRESS */
uint8_t broadcastAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x00}; // Address of Master Server
uint8_t setMACAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, MACAD};

/* ESP Async Timer */
AsyncTimer asynctimer;

/* WiFi Credentials */
const char* ssid = WIFI_SSID; // SSID
const char* password = WIFI_PASS; // Password

/* ESP-NOW Structures */
typedef struct dataPacket {
int trigger = 0;
} dataPacket;

dataPacket sData; // data to send
dataPacket rData; // data to recieve

/*Pin Variables*/
int Onboard_LED = 2;  //Onboard blue LED pin 
int ReedSwitchPin_1 = 4; //Reed switch output 1
int ReedSwitchPin_2 = 13; //Reed switch output 2
int ReedSwitchPin_3 = 14; //Reed switch output 3
int ReedSwitchPin_4 = 16; //Reed switch output 4
int ReedSwitchPin_5 = 17; //Reed switch output 5
int ReedSwitchPin_6 = 18; //Reed switch output 6
int ReedSwitchPin_7 = 19; //Reed switch output 7
int ReedSwitchPin_8 = 21; //Reed switch output 8
int ReedSwitchPin_9 = 22; //Reed switch output 9
int ReedSwitchPin_10 = 23; //Reed switch output 10
int ReedSwitchPin_11 = 25; //Reed switch output 11
int ReedSwitchPin_12= 26; //Reed switch output 12
int ReedSwitchPin_13 = 27; //Reed switch output 13
int ReedSwitchPin_14 = 32; //Reed switch output 14
int ReedSwitchPin_15 = 33; //Reed switch output 15

int MagneticLockRelayPin = 16;  //Maglock relay activation pin 

/*Other Global Variables*/
int PuzzleSolved = 0;
//int PuzzlesolvedPrev = 0;
//int MaglockActivationTrigger = 0;
//int MaglockTimerStart = 0;
//int CurrentMillis = 0;
//int OverrideTrigger = 0;
int ledState = LOW;
unsigned long PreviousMillis = 0;
const long BlinkDelayInterval = 30;
unsigned long CurrentMillis = 0;

/* Setup */
AsyncWebServer server(80);
esp_now_peer_info_t peerInfo;


// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");

}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&rData, incomingData, sizeof(rData));
  Serial.println("Override Data Recieved...");

  //Incoming Data is copied to rData. Do something with it here or in the main loop.
  //Incoming Data Goes Here


}

void startespnow(){
    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
      return;
    }

    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(OnDataSent);
    
    // Register peer
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    
    // Add peer        
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
      Serial.println("Failed to add peer");
      return;
    }
    // Register for a callback function that will be called when data is received
    esp_now_register_recv_cb(OnDataRecv);
}

void startup(){
  /* Connect WiFi */
  WiFi.mode(WIFI_STA);
  esp_wifi_set_mac(WIFI_IF_STA, &setMACAddress[0]);
  
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

  /* MDNS */
  if (!MDNS.begin(NAME)) {
        Serial.println("Error setting up MDNS responder!");
        while(1) {
            delay(1000);
        }
    }
  Serial.println("mDNS responder started");
  
  AsyncElegantOTA.begin(&server, "admin", "admin1234");

  server.begin();
  MDNS.addService("http", "tcp", 80);

}

void Blink(){ //Function that blinks the blue LED onboard the ESP32
if (CurrentMillis - PreviousMillis >= BlinkDelayInterval) {
    // save the last time you blinked the LED
    PreviousMillis = CurrentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(Onboard_LED, ledState);
  }
}

int IsPuzzleSolved(){ //Function that reads all the reed switch inputs and determines if puzzle is solved or not
  if(digitalRead(ReedSwitchPin_1)==1){ Serial.println("Reed1: ON");
    if(digitalRead(ReedSwitchPin_2)==1){Serial.println("Reed2: ON");
      if(digitalRead(ReedSwitchPin_3)==1){Serial.println("Reed3: ON");
        if(digitalRead(ReedSwitchPin_4)==1){Serial.println("Reed4: ON");         
          if(digitalRead(ReedSwitchPin_5)==1){Serial.println("Reed5: ON");
            if(digitalRead(ReedSwitchPin_6)==1){Serial.println("Reed6: ON");
              if(digitalRead(ReedSwitchPin_7)==1){Serial.println("Reed7: ON");
                if(digitalRead(ReedSwitchPin_8)==1){Serial.println("Reed8: ON");
                  if(digitalRead(ReedSwitchPin_9)==1){Serial.println("Reed9: ON");
                    if(digitalRead(ReedSwitchPin_10)==1){Serial.println("Reed10: ON");
                      if(digitalRead(ReedSwitchPin_11)==1){Serial.println("Reed11: ON");
                        if(digitalRead(ReedSwitchPin_12)==1){Serial.println("Reed12: ON");
                          if(digitalRead(ReedSwitchPin_13)==1){Serial.println("Reed13: ON");
                            if(digitalRead(ReedSwitchPin_14)==1){Serial.println("Reed14: ON");
                              if(digitalRead(ReedSwitchPin_15)==1){Serial.println("Reed15: ON");
                                Blink();         
                                return 1;
                              }else{return 0;}
                            }else{return 0;}
                          }else{return 0;}
                        }else{return 0;}
                      }else{return 0;}
                    }else{return 0;}
                  }else{return 0;}
                }else{return 0;}              
              }else{return 0;}
            }else{return 0;}
          }else{return 0;}
        }else{return 0;}
      }else{return 0;}
    }else{return 0;}
  }else{return 0;}  
}

void setup() {

  Serial.begin(115200);
  startup(); // Startup for Wifi, mDNS, and OTA
  startespnow(); // Startup for ESP-NOW
  pinMode(Onboard_LED,OUTPUT); //Onboard LED pin configuration
  pinMode(ReedSwitchPin_1, INPUT);  //Use Pull-Down resistor configuration with GPIO pin as digital input
  pinMode(ReedSwitchPin_2, INPUT);  //Use Pull-Down resistor configuration with GPIO pin as digital input
  pinMode(ReedSwitchPin_3, INPUT);  //Use Pull-Down resistor configuration with GPIO pin as digital input
  pinMode(ReedSwitchPin_4, INPUT);  //Use Pull-Down resistor configuration with GPIO pin as digital input
  pinMode(ReedSwitchPin_5, INPUT);  //Use Pull-Down resistor configuration with GPIO pin as digital input
  pinMode(ReedSwitchPin_6, INPUT);  //Use Pull-Down resistor configuration with GPIO pin as digital input
  pinMode(ReedSwitchPin_7, INPUT);  //Use Pull-Down resistor configuration with GPIO pin as digital input
  pinMode(ReedSwitchPin_8, INPUT);  //Use Pull-Down resistor configuration with GPIO pin as digital input
  pinMode(ReedSwitchPin_9, INPUT);  //Use Pull-Down resistor configuration with GPIO pin as digital input
  pinMode(ReedSwitchPin_10, INPUT);  //Use Pull-Down resistor configuration with GPIO pin as digital input
  pinMode(ReedSwitchPin_11, INPUT);  //Use Pull-Down resistor configuration with GPIO pin as digital input
  pinMode(ReedSwitchPin_12, INPUT);  //Use Pull-Down resistor configuration with GPIO pin as digital input
  pinMode(ReedSwitchPin_13, INPUT);  //Use Pull-Down resistor configuration with GPIO pin as digital input
  pinMode(ReedSwitchPin_14, INPUT);  //Use Pull-Down resistor configuration with GPIO pin as digital input
  pinMode(ReedSwitchPin_15, INPUT);  //Use Pull-Down resistor configuration with GPIO pin as digital input
  //Begin Sending Data to Remote ESP's every 250ms
   asynctimer.setInterval([]() { esp_now_send(broadcastAddress, (uint8_t *) &sData, sizeof(sData));},  250);

}


void loop() {
  //Insert Code Here
  PuzzleSolved=IsPuzzleSolved();
  sData.trigger=PuzzleSolved;


  //You need to edit OnDataRecv to handle incoming overrides.


  asynctimer.handle();
}