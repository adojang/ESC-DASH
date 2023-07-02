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

#define NAME "atticmaster"
#define setMACAddress m_atticmaster

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
// Attic Variables
String success;
int emergencyFlag = 0;
int emergencyTrigger = 0;
int touchtriggertimeout = 0;
const unsigned long emergencyButtonTimeout = 5000;

//WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING 

bool RFID1_status = false;
bool RFID2_status = false;
bool RFID3_status = false;
bool RFID4_status = false;

//WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING 


bool doortimeoutflag = false;
unsigned long doortimeout = 0;

bool DOORTOUCH = false;
bool doorlocked = false;

int readingcounter = 0;

float currentValue = 0.0;
float previousEMA = 0.0;
float prevousEMA2 = 0.0;

float smoothingFactor = 0.8;  // Adjust this value based on your application //0.8 600-3400 floating, 4000 touching

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


  esp_err_t result = esp_now_send(m_masterserver, (uint8_t *) &sData, sizeof(sData));

  if(filteredValue > 3800){
    Serial.printf("ABOVE THRESHOLD\n\n\n\n");
    readingcounter +=80; // trigger in 2 ticks.
  }

if (readingcounter >= 100) {
    readingcounter = 0;
    Serial.println("Touch Detected!");
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
  Serial.println("Door Opened");

  asynctimer.setTimeout([pin]() {
      digitalWrite(pin, HIGH);
      Serial.println("Door Closed");
    }, 3000);
}

void IRAM_ATTR emergency(){
  emergencyFlag = 1;
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {WebSerial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) 
{
  memcpy(&rData, incomingData, sizeof(rData));
  
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
        esp_now_send(m_RFID1, (uint8_t *)&sData, sizeof(sData));
        esp_now_send(m_RFID2, (uint8_t *)&sData, sizeof(sData));
        esp_now_send(m_RFID3, (uint8_t *)&sData, sizeof(sData));
        esp_now_send(m_RFID4, (uint8_t *)&sData, sizeof(sData));

        delay(2000);
        ESP.restart();
    }
    
      if(rData.origin == masterserver && rData.sensor == masterserver && rData.data == 77){
        RFID1_status = false;
        RFID2_status = false;
        RFID3_status = false;
        RFID4_status = false;
        WebSerial.println("RFID Status Reset");
        Serial.println("RFID Status Reset");


// rData.data = 0;
// rData.origin = attic_RFID1;
// rData.sensor = attic_RFID1;
// esp_now_send(m_masterserver, (uint8_t *) &rData, sizeof(rData));
// rData.origin = attic_RFID2;
// rData.sensor = attic_RFID2;
// esp_now_send(m_masterserver, (uint8_t *) &rData, sizeof(rData));
// rData.origin = attic_RFID3;
// rData.sensor = attic_RFID3;
// esp_now_send(m_masterserver, (uint8_t *) &rData, sizeof(rData));
// rData.origin = attic_RFID4;
// rData.sensor = attic_RFID4;
// esp_now_send(m_masterserver, (uint8_t *) &rData, sizeof(rData));




  }
    // Forward Data from Sensors to Master Server
    if (rData.origin != masterserver){
        esp_err_t result = esp_now_send(m_masterserver, (uint8_t *) &rData, sizeof(rData));
    }

    /*    STATUS UPDATES    */
    if(rData.origin == attic_RFID1 && rData.sensor == attic_RFID1 && rData.data == 1){
    RFID1_status = true;
    //Send an Updated 'flag' to master
    rData.data = 100;
    esp_err_t result = esp_now_send(m_masterserver, (uint8_t *) &rData, sizeof(rData));
  }

    if(rData.origin == attic_RFID2 && rData.sensor == attic_RFID2 && rData.data == 2){
    RFID2_status = true;
    //Send an Updated 'flag' to master
    rData.data = 100;
    esp_err_t result = esp_now_send(m_masterserver, (uint8_t *) &rData, sizeof(rData));
  }

    if(rData.origin == attic_RFID3 && rData.sensor == attic_RFID3 && rData.data >= 3){
    RFID3_status = true;
    //Send an Updated 'flag' to master
    rData.data = 100;
    esp_err_t result = esp_now_send(m_masterserver, (uint8_t *) &rData, sizeof(rData));
  }

    if(rData.origin == attic_RFID4 && rData.sensor == attic_RFID4 && rData.data >= 6){
    RFID4_status = true;
    //Send an Updated 'flag' to master
    rData.data = 100;
    esp_err_t result = esp_now_send(m_masterserver, (uint8_t *) &rData, sizeof(rData));
  }

    if (rData.origin == masterserver && rData.sensor == attic_humanchain && rData.data){
      triggerDoor(5);
      Serial.println("Master Server Override");
      WebSerial.println("Master Server Override");
    //   triggerDoor(18, 2000);
    //   triggerDoor(19, 2000);
    //   triggerDoor(21, 2000);
    }

    if (rData.origin == masterserver && rData.sensor == attic_bike){
    if (rData.data == 1) ledcWrite(1, (4000));

    if(rData.data == 0) ledcWrite(1, (4000));
}

    if(rData.origin == attic_bike && rData.sensor == attic_bike)
    {
    //Send Data to LED to light up appropriately
      //4000 / 100 = 40
      ledcWrite(1, ((40)*rData.data));
      // Serial.println(rData.data);

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
}

void setup() {
  Serial.begin(115200);
  
  Core.startup(setMACAddress, NAME, server);
  startespnow();
  registermac(m_masterserver);
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
  digitalWrite(18, HIGH);
  delay(250);
  digitalWrite(19, HIGH);
  delay(250);
  digitalWrite(21, HIGH);

 attachInterrupt(17, emergency, HIGH);

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
  pinMode(32, INPUT_PULLDOWN); // Does not have pullup or pulldown.
  pinMode(15, OUTPUT);
  digitalWrite(15,HIGH);




  #pragma endregion gpio


asynctimer.setInterval([]() {getTouch();},  250);


asynctimer.setInterval([]() {statusUpdate();},  1000);



// delay(2000);

// rData.data = 100;
// rData.origin = attic_RFID1;
// rData.sensor = attic_RFID1;
// esp_now_send(m_masterserver, (uint8_t *) &rData, sizeof(rData));
// rData.origin = attic_RFID2;
// rData.sensor = attic_RFID2;
// esp_now_send(m_masterserver, (uint8_t *) &rData, sizeof(rData));
// rData.origin = attic_RFID3;
// rData.sensor = attic_RFID3;
// esp_now_send(m_masterserver, (uint8_t *) &rData, sizeof(rData));
// rData.origin = attic_RFID4;
// rData.sensor = attic_RFID4;
// esp_now_send(m_masterserver, (uint8_t *) &rData, sizeof(rData));




emergencyTrigger = millis();
touchtriggertimeout = millis();
}



void loop() {


if (millis() - ttime > 2000){ //Use this to print data regularly
  ttime = millis();
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
if ((millis() - touchtriggertimeout > 2000) && (DOORTOUCH == true) && (RFID2_status == true) && (RFID3_status == true) && (RFID4_status == true) && (doorlocked == false)){
    DOORTOUCH = false;
    touchtriggertimeout = millis();
    Serial.println("Human Chain Puzzle Completed");
    WebSerial.println("Human Chain Puzzle Completed. Opening Doors");
    triggerDoor(5);
        
        

    // delay(100);
    // triggerDoor(18, 2000);
    // delay(100);
    // triggerDoor(19, 2000);
    // delay(100);
    // triggerDoor(21, 2000);
  }



//Emergency Escape Button
if ((digitalRead(23)) && (millis() - emergencyTrigger) >= 3000)
  {
    emergencyTrigger = millis();
    emergencyFlag = 0;
    Serial.println("Emergency Button Pushed");
    WebSerial.println("Emergency Button Pushed");
    triggerDoor(5);
    // triggerDoor(18, 2000);
    // triggerDoor(19, 2000);
    // triggerDoor(21, 2000);
  }



  asynctimer.handle();
}