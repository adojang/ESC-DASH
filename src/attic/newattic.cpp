/*
--------------------------------------------------------------------------
                          Tygervallei Escape Room Project
--------------------------------------------------------------------------                          
  Author: Adriaan van Wijk
  Date: 22 January

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
#include <config.h>
#include <esp_task_wdt.h> // watchdog for doorlock mag recovery.
#define NAME "atticmaster"
#define setMACAddress m_atticmaster
#define WDT_TIMEOUT 15 // 15 seconds
#pragma region mac
// Control
uint8_t m_masterserver[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x00};
uint8_t m_trainmaster[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x01};
uint8_t m_tombmaster[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x02};
uint8_t m_atticmaster[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x03};
uint8_t m_atticreset[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x04};
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
// Attic Variables
String success;
unsigned long emergencyTrigger = 0;
int touchtriggertimeout = 0;
const unsigned long emergencyButtonTimeout = 5000;

bool RFID1_status = false;
bool RFID2_status = false;
bool RFID3_status = false;
bool RFID4_status = false;



bool doortimeoutflag = false;
unsigned long doortimeout = 0;

bool DOORTOUCH = false;
bool doorlocked = false;

int readingcounter = 0;

float currentValue = 0.0;
float previousEMA = 0.0;
float prevousEMA2 = 0.0;
bool bike_enabled = true;

float smoothingFactor = 0.8;  // Adjust this value based on your application //0.8 600-3400 floating, 4000 touching

const int defaultDoorTimeout = 2000;


/* Functions */
float emaFilter(float currentValue, float previousEMA, float smoothingFactor) {
  return (currentValue * smoothingFactor) + (previousEMA * (1 - smoothingFactor));
}

void getTouch(){

  int reading = analogRead(34);
  float filteredValue = emaFilter(reading, previousEMA, smoothingFactor);
  previousEMA = filteredValue;
  sData.data = filteredValue;
  sData.origin = attic_humanchain;
  sData.sensor = attic_humanchain;

  if(filteredValue > 3800){
    Serial.printf("ABOVE THRESHOLD\n\n\n\n");
    //baseline was 80
    readingcounter +=30; // trigger in 4 ticks.
  }

if (readingcounter >= 100) {
    readingcounter = 0;
    WebSerial.printf("Touch Detected!\n\n");
    DOORTOUCH = true;
    doortimeoutflag = true;
    doortimeout = millis();

    sData.origin = atticmaster;
    sData.sensor = attic_humanchain;
    sData.data = 1;
    esp_err_t result = esp_now_send(m_masterserver, (uint8_t *) &sData, sizeof(sData));
}
  readingcounter -= 5;
  if (readingcounter < 0) readingcounter = 0;
}

void triggerDoor(int pin){
  digitalWrite(pin, LOW);
  WebSerial.println("Door Opened");


  asynctimer.setTimeout([pin]() {
      digitalWrite(pin, HIGH);
      WebSerial.println("Door Closed");
    }, defaultDoorTimeout);
}


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // WebSerial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");
  }



void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) 
{
  memcpy(&rData, incomingData, sizeof(rData));


    if(rData.origin == masterserver && rData.sensor == masterserver && rData.data == 999){
      while(true){
        Serial.println("Crashed");
        delay(500);
      }
    }


    if(rData.origin == masterserver && rData.sensor == attic_bike){
      
      if(rData.data == 1){ 
      
      ledcWrite(1, (4000));
      digitalWrite(2,HIGH);
      //DISABLE THE BIKE LIGHT MESSAGES
      bike_enabled = false;

      }
      if(rData.data == 0) {
        digitalWrite(2,LOW);
        ledcWrite(1, (0));

        //ENABLE BIKE LIGHT MESSAGES
      bike_enabled = true;

      }
    }


    if(rData.origin == masterserver && rData.sensor == masterserver && rData.data == 50){
      //Override RFID

      RFID1_status = true;
      RFID2_status = true;
      RFID3_status = true;
      RFID4_status = true;

    rData.data = 100;
    rData.origin = attic_RFID1;
    rData.sensor = attic_RFID1;
    esp_now_send(m_masterserver, (uint8_t *) &rData, sizeof(rData));
    rData.origin = attic_RFID2;
    rData.sensor = attic_RFID2;
    esp_now_send(m_masterserver, (uint8_t *) &rData, sizeof(rData));
    rData.origin = attic_RFID3;
    rData.sensor = attic_RFID3;
    esp_now_send(m_masterserver, (uint8_t *) &rData, sizeof(rData));
    rData.origin = attic_RFID4;
    rData.sensor = attic_RFID4;
    esp_now_send(m_masterserver, (uint8_t *) &rData, sizeof(rData));

    }

    if (rData.origin == masterserver && rData.sensor == masterserver)
    {
        if (rData.data == 88){doorlocked = true;} // locked
            
        if (rData.data == 99){doorlocked = false;} // unlocked
            
    }

    if ((rData.origin == masterserver) && (rData.data == 66))
    { // Restart Attic and RFIDs
        sData.origin = atticmaster;
        sData.sensor = atticmaster;
        sData.data = 66;
         esp_err_t err1 = esp_now_send(m_RFID1, (uint8_t *)&sData, sizeof(sData));
         esp_err_t err2 = esp_now_send(m_RFID2, (uint8_t *)&sData, sizeof(sData));
         esp_err_t err3 = esp_now_send(m_RFID3, (uint8_t *)&sData, sizeof(sData));
        esp_err_t err4 = esp_now_send(m_RFID4, (uint8_t *)&sData, sizeof(sData));

        if ((err1 == ESP_OK) && (err2 == ESP_OK) && (err3 == ESP_OK) && (err4 == ESP_OK)){
            ESP.restart();
        }


    }
    
      if(rData.origin == masterserver && rData.sensor == masterserver && rData.data == 77){
        RFID1_status = false;
        RFID2_status = false;
        RFID3_status = false;
        RFID4_status = false;
        WebSerial.println("RFID Status Reset");
        Serial.println("RFID Status Reset");
  }
    // Forward Data from Sensors to Master Server
    if (rData.origin != masterserver && rData.origin != attic_bike){
      // WebSerial.println("Forward Data to Master (Proably from RFIDs)");   
        esp_err_t result = esp_now_send(m_masterserver, (uint8_t *) &rData, sizeof(rData));
        if (result != ESP_OK) {
          // WebSerial.println("Error sending the data");
        }
    }

    /*    STATUS UPDATES    */
    if(rData.origin == attic_RFID1 && rData.sensor == attic_RFID1){
    if (rData.data == 1) {
      RFID1_status = true;
    }
    else {
      RFID1_status = false;
    }
    //Send an Updated 'flag' to master
    // rData.data = 100;
    esp_err_t result = esp_now_send(m_masterserver, (uint8_t *) &rData, sizeof(rData));
  }

    if(rData.origin == attic_RFID2 && rData.sensor == attic_RFID2){

        if (rData.data == 2) {
      RFID2_status = true;
    }
    else {
      RFID2_status = false;
    }
    
    //Send an Updated 'flag' to master
    // rData.data = 100;
    esp_err_t result = esp_now_send(m_masterserver, (uint8_t *) &rData, sizeof(rData));
  }


    if(rData.origin == attic_RFID3 && rData.sensor == attic_RFID3){
        if (rData.data == 3) {
      RFID3_status = true;
    }
    else {
      RFID3_status = false;
    }

    //Send an Updated 'flag' to master
    esp_err_t result = esp_now_send(m_masterserver, (uint8_t *) &rData, sizeof(rData));
  }

    if(rData.origin == attic_RFID4 && rData.sensor == attic_RFID4){
        if (rData.data == 5) {
      RFID4_status = true;
    }
    else {
      RFID4_status = false;
    }
    //Send an Updated 'flag' to master
    esp_err_t result = esp_now_send(m_masterserver, (uint8_t *) &rData, sizeof(rData));
  }

    if (rData.origin == masterserver && rData.sensor == attic_humanchain && rData.data){
      emergencyTrigger = millis();
      triggerDoor(5);
      WebSerial.println("Master Server Override");
      emergencyTrigger = millis();
    }


    if(rData.origin == attic_bike && rData.sensor == attic_bike)
    {
    //Send Data to LED to light up appropriately
      //4000 / 100 = 40
      if(bike_enabled){
        ledcWrite(1, ((40)*rData.data));
        // Serial.println(rData.data);

      }



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
  sData.origin = atticmaster;
  sData.sensor = status_alive;
  esp_err_t result = esp_now_send(m_masterserver, (uint8_t *) &sData, sizeof(sData));
  esp_err_t result2 = esp_now_send(m_atticreset, (uint8_t *) &sData, sizeof(sData));
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
  registermac(m_atticreset); // for external watchdog
  registermac(m_clock);
  registermac(m_RFID1);
  registermac(m_RFID2);
  registermac(m_RFID3);
  registermac(m_RFID4);
  

  #pragma region gpio
  
  pinMode(22, OUTPUT); // emergency Button GND
  digitalWrite(22, HIGH);
  pinMode(23, INPUT_PULLDOWN); // emergency Button
  pinMode(2, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(18, OUTPUT);
  pinMode(19, OUTPUT);
  pinMode(21, OUTPUT);
  digitalWrite(5, HIGH);
  delay(250);
  digitalWrite(18, LOW);
  delay(250);
  digitalWrite(19, LOW);
  delay(250);
  digitalWrite(21, HIGH);

  // BIKE LIGHT
  ledcSetup(1, 100, 12); // Bike LED
  ledcAttachPin(26, 1);
  ledcWrite(1, 0);

  analogReadResolution(12);

  //GND Pin for PWM Controllers
  pinMode(33, OUTPUT);
  digitalWrite(33, LOW);
  pinMode(25, OUTPUT);
  digitalWrite(25, LOW);

  //HumanTouch Pins
  pinMode(34, INPUT);
  pinMode(15, OUTPUT);
  digitalWrite(15,HIGH);




  #pragma endregion gpio


asynctimer.setInterval([]() {getTouch();},  250);

asynctimer.setInterval([]() {statusUpdate();},  1000);

//Get Data from RFID Tag Sensors (REPEAT every 2 seconds)
  // asynctimer.setInterval([]() {
    rData.data = 0;
    rData.origin = atticmaster;
    rData.sensor = atticmaster;

    //Request Data from the RFID controller.

    esp_now_send(m_RFID1, (uint8_t *) &rData, sizeof(rData));
    esp_now_send(m_RFID2, (uint8_t *) &rData, sizeof(rData));
    esp_now_send(m_RFID3, (uint8_t *) &rData, sizeof(rData));
    esp_now_send(m_RFID4, (uint8_t *) &rData, sizeof(rData));
    // }, 2000);


emergencyTrigger = millis();
WebSerial.println("End of Setup");
touchtriggertimeout = millis();
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println(info.wifi_sta_disconnected.reason);
  Serial.println("Trying to Reconnect");
}


void loop() {

if (WiFi.isConnected() == false){
  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  
  Serial.println("Start Reconnect Process");
  WiFi.softAPdisconnect();
  WiFi.disconnect();

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  delay(500);
  while(!WiFi.isConnected()){
    Serial.println("Waiting Forever...");
    delay(200);
  }

}

//2 Second Timeout for Doortouch
if (doortimeoutflag == true && (millis() - doortimeout > 2000)){
    Serial.println("DoorTouch Timeout. Now False");
    WebSerial.println("DoorTouch Timeout. Now False");
    doortimeoutflag = false;
    DOORTOUCH = false;

    sData.origin = atticmaster;
    sData.sensor = attic_humanchain;
    sData.data = 0;
    esp_err_t result = esp_now_send(m_masterserver, (uint8_t *) &sData, sizeof(sData));
}

//The Timeout here is to prevent over triggering.
if ((millis() - touchtriggertimeout > 2000) && (DOORTOUCH == true) && (RFID1_status == true) && (RFID2_status == true) && (RFID3_status == true) && (RFID4_status == true) && (doorlocked == false)){
    DOORTOUCH = false;
    touchtriggertimeout = millis();
    WebSerial.println("Human Chain Puzzle Completed. Opening Door");
    emergencyTrigger = millis(); // Weird relay back emf workaround
    triggerDoor(5);
    emergencyTrigger = millis(); // Weird relay back emf workaround
        
        

    // delay(100);
    // triggerDoor(18, 2000);
    // delay(100);
    // triggerDoor(19, 2000);
    // delay(100);
    // triggerDoor(21, 2000);
  }

  asynctimer.handle();
}





