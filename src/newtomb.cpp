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
#include <esp_task_wdt.h> // watchdog for doorlock mag recovery if it get stuck

#define NAME "tombmaster"
#define setMACAddress m_tombmaster
#define WDT_TIMEOUT 10 // 10 seconds

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
unsigned long emergencyTrigger = millis();
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


  // esp_err_t result = esp_now_send(m_masterserver, (uint8_t *) &sData, sizeof(sData));

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
  digitalWrite(pin, HIGH);
  Serial.println("Door Opened");
  emergencyTrigger = millis(); // to prevent unwanted emf from accidently triggering.

  asynctimer.setTimeout([pin]() {
      digitalWrite(pin, LOW);
      Serial.println("Door Closed");
      emergencyTrigger = millis(); // to prevent unwanted emf from accidently triggering.
    }, 3000);
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // WebSerial.println(status == ESP_NOW_SEND_SUCCESS ? "Packet Delivery Success" : "Packet Delivery Fail");
  }

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) 
{
  memcpy(&rData, incomingData, sizeof(rData));
  
    if (rData.origin == tomb_chalice && rData.sensor == tomb_chalice && rData.data == 1){
    emergencyTrigger = millis();
    emergencyFlag = 0;
    Serial.println("Chalice Button Pushed");
    WebSerial.println("Chalice Button Pushed");
    triggerDoor(19);
    emergencyTrigger = millis(); // Weird relay back emf workaround
    }

        if (rData.origin == masterserver && rData.sensor == tomb_chalice && rData.data == 1){
    emergencyTrigger = millis();
    emergencyFlag = 0;
    Serial.println("Sliding Door Override");
    WebSerial.println("Sliding Door Override");
    triggerDoor(19);
    emergencyTrigger = millis(); // Weird relay back emf workaround
    }

    if ((rData.origin == masterserver) && (rData.data == 66))
    { // Restart Tomb and RFIDs

        delay(2000);
        ESP.restart();
    }
    
    if (rData.origin == masterserver && rData.sensor == masterserver && rData.data == 111){
      emergencyTrigger = millis();
      triggerDoor(5);
      Serial.println("Master Server Main Door Override");
      WebSerial.println("Master Server Main Door Override");
      emergencyTrigger = millis();
    }

    if (rData.origin == masterserver && rData.sensor == tomb_slidedoorOverride && rData.data){
      emergencyTrigger = millis();
      triggerDoor(19);
      Serial.println("Master Server Override");
      WebSerial.println("Master Server Override");
      emergencyTrigger = millis();
    }

    if (rData.origin == masterserver && rData.sensor == tomb_tangrum && rData.data && rData.data == 1){
      emergencyTrigger = millis();
      triggerDoor(18);
      Serial.println("Master Server Tomb Override");
      WebSerial.println("Master Server Tomb Override");
      emergencyTrigger = millis();
    }

      if (rData.origin == tomb_tangrum && rData.sensor == tomb_tangrum && rData.data && rData.data == 1){
      emergencyTrigger = millis();
      triggerDoor(18);
      Serial.println("Tangrum Puzzle Triggered, Open Tomb");
      WebSerial.println("Tangrum Puzzle Triggered, Open Tomb");
      emergencyTrigger = millis();
    }


    // Forward Data from Sensors to Master Server
    if (rData.origin != masterserver){
      WebSerial.println("Forward Data to Master (Proably from RFIDs)");   
        esp_err_t result = esp_now_send(m_masterserver, (uint8_t *) &rData, sizeof(rData));
        if (result != ESP_OK) {
          WebSerial.println("Error sending the data");
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
  sData.origin = tombmaster;
  sData.sensor = status_alive;
  esp_err_t result = esp_now_send(m_masterserver, (uint8_t *) &sData, sizeof(sData));
  esp_task_wdt_reset(); //restarts out after 10 seconds of not sending.
}

void setup() {
  Serial.begin(115200);
  
  Core.startup(setMACAddress, NAME, server);
  startespnow();
  registermac(m_masterserver);
  registermac(m_tomb_chalice);
  registermac(m_tomb_sennet);
  registermac(m_tomb_tangrum);

  Serial.println("Configuring WDT...");
  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch

  #pragma region gpio
  
  pinMode(22, OUTPUT); // out emergency Button 3.3V
  digitalWrite(22, HIGH);
  pinMode(23, INPUT_PULLDOWN); // emergency Button OUT


// Changed for 25 and 26 the pins may have issues 12 must be output.
  pinMode(16, OUTPUT); // IN emergency Button 3.3V
  digitalWrite(16, HIGH);
  pinMode(13, INPUT_PULLDOWN); // emergency Button IN


  //Relay Outputs
  pinMode(2, OUTPUT);
  pinMode(5, OUTPUT); // Front Door
  pinMode(19, OUTPUT); // TOMB
  pinMode(18, OUTPUT); // Inner Door
  pinMode(21, OUTPUT);
  
  //Startup Sequence
  digitalWrite(5, LOW);
  delay(250);
  digitalWrite(19, LOW);
  delay(250);
  digitalWrite(18, LOW);
  delay(250);
  digitalWrite(21, LOW);
  #pragma endregion gpio



asynctimer.setInterval([]() {statusUpdate();},  1000);


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

int digitalDebounce(int pin){
  int value = 0;
  if (digitalRead(pin)){ // Takes 50ms
    for (int i = 0; i < 10; i++){
      if (digitalRead(pin) == 1) value++;
      delay(5);
    }
  }

  if( value > 8) return 1; // 80% sure it was a button press
    else { // 8 or less
          if (value == 0) return 0; // 0
            else{ // 8 or less
            Serial.println("Emergency Button was true, but debounce was false");
            WebSerial.println("Emergency Button was true, but debounce was false");
            return 0;
          }
    }
}

void loop() {



if (millis() - ttime > 2000){ //Use this to print data regularly
  ttime = millis();
  WebSerial.printf("Button: %d\n Emergency Trigger: %lu\n Millis: %lu\n",digitalDebounce(23),emergencyTrigger,millis());
}



//Emergency Escape Button Entrance Door
if ((digitalDebounce(23)) && (millis() - emergencyTrigger) >= 3000)
  {
    emergencyTrigger = millis();
    emergencyFlag = 0;
    Serial.println("Emergency Button Pushed Entrance");
    WebSerial.println("Emergency Button Pushed Entrance");
    triggerDoor(5);
    emergencyTrigger = millis(); // Weird relay back emf workaround
  }

  //Emergency Escape Button Sliding Door
  if ((digitalDebounce(13)) && (millis() - emergencyTrigger) >= 3000)
  {
    emergencyTrigger = millis();
    emergencyFlag = 0;
    Serial.println("Emergency Button Pushed Sliding Door");
    WebSerial.println("Emergency Button Pushed Sliding Door");
    triggerDoor(19);
    emergencyTrigger = millis(); // Weird relay back emf workaround
  }


  if (WiFi.isConnected() == false){
  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  
  WebSerial.println("Start Reconnect Process");
  digitalWrite(2,HIGH);
  WiFi.softAPdisconnect();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(1000);
  WiFi.softAP(NAME, "pinecones", 0, 1, 4);
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  delay(1000);
  startespnow();
  digitalWrite(2,HIGH);
  while(!WiFi.isConnected()){
    WebSerial.println("Waiting Forever...");
    delay(200);
    digitalWrite(2,LOW);
  }
}




  asynctimer.handle();
}